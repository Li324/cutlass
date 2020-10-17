/***************************************************************************************************
 * Copyright (c) 2017-2020, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the names of its
 *contributors may be used to endorse or promote products derived from this
 *software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY DIRECT,
 *INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TOR (INCLUDING
 *NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************************************/
/*! \file
    \brief This defines a "fragment" iterator for visiting the fragments of an
   accumulator tile that participate in one warp-level store operation.

      Typically, the accumulator tile is the largest single block of
   register-backed storage within the kernel. Storing it to memory is best
   accomplished by partitioning it into smaller tiles and storing these
   sequentially.

      Round trips through shared memory during the Epilogue phase require
   partitioning, as shared memory capacity is typically insufficient for a
   threadblock's total accumulator size.
*/

/**
 * \file include/cutlass/epilogue/warp/fragment_iterator_simt.h
 *
 * Copyright (c) 2014-2020 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * This file has been modified by Megvii ("Megvii Modification"). 
 * All Megvii Modifications are Copyright (C) 2014-2020 Megvii Inc. All rights reserved.
 */
#pragma once

#include "cutlass/array.h"
#include "cutlass/layout/matrix.h"

#include "cutlass/epilogue/warp/simt_policy.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace cutlass {
namespace epilogue {
namespace warp {

/////////////////////////////////////////////////////////////////////////////////////////////////

/// Fragment iterator for SIMT accumulator arrangements
template <typename WarpShape,      ///< shape of warp-level GEMM (concept:
                                   ///< MatrixShape)
          typename Operator,       ///< matrix multiply operation (concept:
                                   ///< arch::Mma)
          typename Layout,         ///< target shared memory layout
          typename MmaSimtPolicy,  ///< policy defining lane arrangement
                                   ///< (concept: MmaSimtPolicy)
          typename Policy = SimtPolicy<WarpShape, Operator, Layout,
                                       MmaSimtPolicy>  ///< Policy
          >
class FragmentIteratorSimt;

/////////////////////////////////////////////////////////////////////////////////////////////////

/// Partial specialization for row-major shared memory
template <typename WarpShape_,      ///< shape of the warp-level GEMM tile
          typename Operator_,       ///< matrix multiply operator (concept:
                                    ///< arch::Mma)
          typename Layout_,         ///< target shared memory layout
          typename MmaSimtPolicy_,  ///< policy defining lane arrangement
                                    ///< (concept: MmaSimtPolicy)
          typename Policy_>
class FragmentIteratorSimt {
public:
    using WarpShape = WarpShape_;
    using Operator = Operator_;
    using Layout = Layout_;

    /// Policy for warp-level epilogue components
    using Policy = Policy_;

    /// This is the fragment size produced by one access of the iterator.
    using Fragment =
            Array<typename Operator::ElementC, Policy::kElementsPerIteration>;

    /// This is the complete warp-level accumulator tile.
    using AccumulatorTile = Array<typename Operator::ElementC,
                                  Policy::kAccumulatorElementCount>;

    using OutputAccumulatorTile = AccumulatorTile;

    /// Number of times this iterator can be incremented
    static int const kIterations = Policy::kIterations;

private:
    /// Internal access type
    using AccessType =
            Array<typename Operator::ElementC, Policy::kElementsPerAccess>;

private:
    //
    // Data members
    //

    /// Accumulator tile
    AccessType const* accumulators_;

    /// Internal index
    int index_;

public:
    /// Constructs an iterator
    CUTLASS_HOST_DEVICE
    FragmentIteratorSimt(AccumulatorTile const& accum)
            : accumulators_(reinterpret_cast<AccessType const*>(&accum)),
              index_(0) {}

    /// Increments
    CUTLASS_HOST_DEVICE
    FragmentIteratorSimt& operator++() {
        ++index_;
        return *this;
    }

    /// Decrements
    CUTLASS_HOST_DEVICE
    FragmentIteratorSimt& operator--() {
        --index_;
        return *this;
    }

    /// Loads a fragment from the referenced part of the accumulator tile
    CUTLASS_HOST_DEVICE
    void load(Fragment& frag, int index_offset = 0) const {
        AccessType* frag_ptr = reinterpret_cast<AccessType*>(&frag);

        CUTLASS_PRAGMA_UNROLL
        for (int n = 0; n < Policy::kAccessesPerIteration; ++n) {
            int accumulator_access_offset =
                    index_ * Policy::kAccessesPerIteration + n;

            frag_ptr[n] = accumulators_[accumulator_access_offset];
        }
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace warp
}  // namespace epilogue
}  // namespace cutlass

/////////////////////////////////////////////////////////////////////////////////////////////////
