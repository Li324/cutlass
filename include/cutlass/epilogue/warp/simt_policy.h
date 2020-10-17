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
    \brief Defines basic structures needed for implementing the warp-scoped
   phase of the epilogue. These quantities assume a 'column-major' arrangement
   of SimtOp instructions, of which a row-oriented slice is visible per
   iteration.
*/

#pragma once

#include "cutlass/layout/matrix.h"
#include "cutlass/matrix_shape.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace cutlass {
namespace epilogue {
namespace warp {

/////////////////////////////////////////////////////////////////////////////////////////////////

template <
        typename WarpShape,  ///< shape of warp-level GEMM (concept: GemmShape)
        typename Operator,   ///< matrix multiply operation (concept: arch::Mma)
        typename Layout,     ///< destination layout in shared memory
        typename MmaSimtPolicy  ///< policy defining lane arrangement (concept:
                                ///< MmaSimtPolicy)
        >
struct SimtPolicy;

/////////////////////////////////////////////////////////////////////////////////////////////////

/// Partial specialization for row-major
template <typename WarpShape_,     ///< shape of warp-level GEMM (concept:
                                   ///< MatrixShape)
          typename Operator_,      ///< matrix multiply operation (concept:
                                   ///< arch::Mma)
          typename MmaSimtPolicy_  ///< policy defining lane arrangement
                                   ///< (concept: MmaSimtPolicy)
          >
struct SimtPolicy<WarpShape_, Operator_, layout::RowMajor, MmaSimtPolicy_> {
    using WarpShape = WarpShape_;
    using Operator = Operator_;
    using MmaSimtPolicy = MmaSimtPolicy_;

    static_assert(!(WarpShape::kM % MmaSimtPolicy::WarpShape::kRow),
                  "Divisibility");
    static_assert(!(WarpShape::kN % MmaSimtPolicy::WarpShape::kColumn),
                  "Divisibility");

    /// Number of iterations
    static int const kIterations =
            WarpShape::kM / MmaSimtPolicy::WarpShape::kRow;

    /// Number of accumulators written per iteration
    static int const kElementsPerIteration =
            (WarpShape::kN / MmaSimtPolicy::WarpShape::kColumn);

    /// Total number of accumulators
    static int const kAccumulatorElementCount =
            kElementsPerIteration * kIterations;

    /// Number of consecutive elements
    static int const kElementsPerAccess = MmaSimtPolicy::LaneMmaShape::kN;

    /// Number of rows per epilogue iteration
    static int const kRowsPerIteration = MmaSimtPolicy::WarpShape::kRow;

    /// Number of accesses made in one iteration
    static int const kAccessesPerIteration =
            kElementsPerIteration / kElementsPerAccess;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace warp
}  // namespace epilogue
}  // namespace cutlass

/////////////////////////////////////////////////////////////////////////////////////////////////
