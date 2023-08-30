// Copyright (c) 2023 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>

#include "paddle/fluid/ir/dialect/paddle_dialect/ir/pd_dialect.h"
#include "paddle/fluid/ir/dialect/paddle_dialect/ir/pd_op.h"
#include "paddle/fluid/ir/drr/api/drr_pattern_base.h"
#include "paddle/ir/pass/pass.h"
#include "paddle/ir/pass/pass_manager.h"
#include "paddle/ir/pattern_rewrite/pattern_rewrite_driver.h"

class MultiHeadMatmulFusePattern
    : public ir::drr::DrrPatternBase<MultiHeadMatmulFusePattern> {
 public:
  void operator()(ir::drr::DrrPatternContext *ctx) const override {
    //
    // Source Pattern.
    //
    ir::drr::SourcePattern src = ctx->SourcePattern();
    // The first path to matmul with scale (q).
    const auto &matmul_1 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_1_transpose_x")},
                {"transpose_y", src.Attr("matmul_1_transpose_y")}});
    src.Tensor("matmul_1_out") =
        matmul_1(src.Tensor("matmul_1_in_1"), src.Tensor("matmul_1_in_2"));
    const auto &add_1 = src.Op("pd.add");
    src.Tensor("add_1_out") =
        add_1(src.Tensor("matmul_1_out"), src.Tensor("add_1_in_2"));
    const auto &full_int_array_1 = src.Op(
        "pd.full_int_array", {{"value", src.Attr("full_int_array_1_value")}});
    const auto &reshape_1 = src.Op("pd.reshape");
    reshape_1({&src.Tensor("add_1_out"), &full_int_array_1()},
              {&src.Tensor("reshape_1_out"), &src.Tensor("reshape_1_xshape")});
    const auto &transpose_1 = src.Op("pd.transpose");
    src.Tensor("transpose_1_out") = transpose_1(src.Tensor("reshape_1_out"));
    const auto &full_1 =
        src.Op("pd.full", {{"value", src.Attr("full_1_value")}});
    const auto &scale = src.Op("pd.scale");
    src.Tensor("scale_out") = scale(src.Tensor("transpose_1_out"), full_1());

    // The second path to matmul (k).
    const auto &matmul_2 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_2_transpose_x")},
                {"transpose_y", src.Attr("matmul_2_transpose_y")}});
    src.Tensor("matmul_2_out") =
        matmul_2(src.Tensor("matmul_1_in_1"), src.Tensor("matmul_2_in_2"));
    const auto &add_2 = src.Op("pd.add");
    src.Tensor("add_2_out") =
        add_2(src.Tensor("matmul_2_out"), src.Tensor("add_2_in_2"));
    const auto &full_int_array_2 = src.Op("pd.full_int_array");
    const auto &reshape_2 = src.Op("pd.reshape");
    reshape_2({&src.Tensor("add_2_out"), &full_int_array_2()},
              {&src.Tensor("reshape_2_out"), &src.Tensor("reshape_2_xshape")});
    const auto &transpose_2 = src.Op("pd.transpose");
    src.Tensor("transpose_2_out") = transpose_2(src.Tensor("reshape_2_out"));

    // The third path to matmul (v).
    const auto &matmul_3 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_3_transpose_x")},
                {"transpose_y", src.Attr("matmul_3_transpose_y")}});
    src.Tensor("matmul_3_out") =
        matmul_3(src.Tensor("matmul_1_in_1"), src.Tensor("matmul_3_in_2"));
    const auto &add_3 = src.Op("pd.add");
    src.Tensor("add_3_out") =
        add_3(src.Tensor("matmul_3_out"), src.Tensor("add_3_in_2"));
    const auto &full_int_array_3 = src.Op("pd.full_int_array");
    const auto &reshape_3 = src.Op("pd.reshape");
    reshape_3({&src.Tensor("add_3_out"), &full_int_array_3()},
              {&src.Tensor("reshape_3_out"), &src.Tensor("reshape_3_xshape")});
    const auto &transpose_3 = src.Op("pd.transpose");
    src.Tensor("transpose_3_out") = transpose_3(src.Tensor("reshape_3_out"));

    // softmax(qk)v + matmul
    const auto &matmul_4 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_4_transpose_x")},
                {"transpose_y", src.Attr("matmul_4_transpose_y")}});
    src.Tensor("matmul_4_out") =
        matmul_4(src.Tensor("scale_out"), src.Tensor("transpose_2_out"));
    const auto &softmax =
        src.Op("pd.softmax", {{"axis", src.Attr("softmax_axis")}});
    src.Tensor("softmax_out") = softmax(src.Tensor("matmul_4_out"));
    const auto &matmul_5 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_5_transpose_x")},
                {"transpose_y", src.Attr("matmul_5_transpose_y")}});
    src.Tensor("matmul_5_out") =
        matmul_5(src.Tensor("softmax_out"), src.Tensor("transpose_3_out"));
    const auto &transpose_4 = src.Op("pd.transpose");
    src.Tensor("transpose_4_out") = transpose_4(src.Tensor("matmul_5_out"));
    const auto &full_int_array_4 = src.Op("pd.full_int_array");
    const auto &reshape_4 = src.Op("pd.reshape");
    reshape_4({&src.Tensor("transpose_4_out"), &full_int_array_4()},
              {&src.Tensor("reshape_4_out"), &src.Tensor("reshape_4_xshape")});
    const auto &matmul_6 =
        src.Op("pd.matmul",
               {{"transpose_x", src.Attr("matmul_6_transpose_x")},
                {"transpose_y", src.Attr("matmul_6_transpose_y")}});
    src.Tensor("matmul_6_out") =
        matmul_6(src.Tensor("reshape_4_out"), src.Tensor("matmul_6_in_2"));
    const auto &add_4 = src.Op("pd.add");
    src.Tensor("add_4_out") =
        add_4(src.Tensor("matmul_6_out"), src.Tensor("add_4_in_2"));

    //
    // Constraints.
    //
    src.RequireNativeCall([](const ir::drr::MatchContext &match_ctx) -> bool {
      const auto &softmax_axis = match_ctx.Attr<int>("softmax_axis");
      if (softmax_axis == -1 || softmax_axis == 3) return false;

      const auto &matmul_1_transpose_x =
          match_ctx.Attr<int>("matmul_1_transpose_x");
      const auto &matmul_1_transpose_y =
          match_ctx.Attr<int>("matmul_1_transpose_y");
      if (matmul_1_transpose_x || matmul_1_transpose_y) return false;

      const auto &matmul_2_transpose_x =
          match_ctx.Attr<int>("matmul_2_transpose_x");
      const auto &matmul_2_transpose_y =
          match_ctx.Attr<int>("matmul_2_transpose_y");
      if (matmul_2_transpose_x || matmul_2_transpose_y) return false;

      const auto &matmul_3_transpose_x =
          match_ctx.Attr<int>("matmul_3_transpose_x");
      const auto &matmul_3_transpose_y =
          match_ctx.Attr<int>("matmul_3_transpose_y");
      if (matmul_3_transpose_x || matmul_3_transpose_y) return false;

      const auto &matmul_4_transpose_x =
          match_ctx.Attr<int>("matmul_4_transpose_x");
      const auto &matmul_4_transpose_y =
          match_ctx.Attr<int>("matmul_4_transpose_y");
      if (matmul_4_transpose_x || !matmul_4_transpose_y) return false;

      const auto &matmul_5_transpose_x =
          match_ctx.Attr<int>("matmul_5_transpose_x");
      const auto &matmul_5_transpose_y =
          match_ctx.Attr<int>("matmul_5_transpose_y");
      if (matmul_5_transpose_x || matmul_5_transpose_y) return false;

      const auto &matmul_6_transpose_x =
          match_ctx.Attr<int>("matmul_6_transpose_x");
      const auto &matmul_6_transpose_y =
          match_ctx.Attr<int>("matmul_6_transpose_y");
      if (matmul_6_transpose_x || matmul_6_transpose_y) return false;

      return true;
    });

    //
    // Result Pattern.
    //
    ir::drr::ResultPattern res = src.ResultPattern();
    const auto &head_number =
        res.Attr([](const ir::drr::MatchContext &match_ctx) -> int {
          const auto &full_int_array_1_value =
              match_ctx.Attr<std::vector<int64_t>>("full_int_array_1_value");
          return full_int_array_1_value.at(2);
        });
    const auto &alpha =
        res.Attr([](const ir::drr::MatchContext &match_ctx) -> float {
          return match_ctx.Attr<float>("full_1_value");
        });
    const auto &multihead_matmul =
        res.Op("pd.multihead_matmul",
               {{"head_number", head_number}, {"alpha", alpha}});
    res.Tensor("add_4_out") = multihead_matmul(res.Tensor("matmul_1_in_1"));
  }
};

class AttentionFusePass : public ir::Pass {
 public:
  AttentionFusePass() : ir::Pass("AttentionFusePass", 1) {}

  bool Initialize(ir::IrContext *context) override {
    ir::RewritePatternSet ps(context);
    ps.Add(MultiHeadMatmulFusePattern().Build(context));
    // Add other attention variant fuse pattern.

    patterns_ = ir::FrozenRewritePatternSet(std::move(ps));
    return true;
  }

  void Run(ir::Operation *op) override {
    ir::GreedyRewriteConfig cfg;
    cfg.use_top_down_traversal = true;
    cfg.max_iterations = 10;
    ir::ApplyPatternsGreedily(op->region(0), patterns_, cfg);
  }

  bool CanApplyOn(ir::Operation *op) const override {
    return op->name() == "builtin.module" && op->num_regions() > 0;
  }

 private:
  ir::FrozenRewritePatternSet patterns_;
};

void BuildProgram(ir::Builder &builder) {  // NOLINT
  paddle::dialect::FullOp full_input_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{1, 300, 256},
                                             0.9,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());

  // left
  paddle::dialect::FullOp full_mat1_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256, 256},
                                             1.1,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());
  paddle::dialect::MatmulOp matmul_op1 =
      builder.Build<paddle::dialect::MatmulOp>(
          full_input_op.out(), full_mat1_y_op.out(), false, false);

  paddle::dialect::FullOp full_eleadd1_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256},
                                             1.5,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());

  paddle::dialect::AddOp add_op1 = builder.Build<paddle::dialect::AddOp>(
      matmul_op1.out(), full_eleadd1_y_op.out());

  paddle::dialect::ReshapeOp reshape_op1 =
      builder.Build<paddle::dialect::ReshapeOp>(
          add_op1.out(), std::vector<int64_t>{1, 300, 8, 32});

  paddle::dialect::TransposeOp transpose_op1 =
      builder.Build<paddle::dialect::TransposeOp>(reshape_op1.out(),
                                                  std::vector<int>{0, 2, 1, 3});

  // middle
  paddle::dialect::FullOp full_mat2_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256, 256},
                                             1.1,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());

  paddle::dialect::MatmulOp matmul_op2 =
      builder.Build<paddle::dialect::MatmulOp>(
          full_input_op.out(), full_mat2_y_op.out(), false, false);

  paddle::dialect::FullOp full_eleadd2_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256},
                                             1.5,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());
  paddle::dialect::AddOp add_op2 = builder.Build<paddle::dialect::AddOp>(
      matmul_op2.out(), full_eleadd2_y_op.out());

  paddle::dialect::ReshapeOp reshape_op2 =
      builder.Build<paddle::dialect::ReshapeOp>(
          add_op2.out(), std::vector<int64_t>{1, 300, 8, 32});

  paddle::dialect::TransposeOp transpose_op2 =
      builder.Build<paddle::dialect::TransposeOp>(reshape_op2.out(),
                                                  std::vector<int>{0, 2, 1, 3});

  // right
  paddle::dialect::FullOp full_mat3_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256, 256},
                                             1.1,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());

  paddle::dialect::MatmulOp matmul_op3 =
      builder.Build<paddle::dialect::MatmulOp>(
          full_input_op.out(), full_mat3_y_op.out(), false, false);

  paddle::dialect::FullOp full_eleadd3_y_op =
      builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256},
                                             1.5,
                                             phi::DataType::FLOAT32,
                                             phi::CPUPlace());

  paddle::dialect::AddOp add_op3 = builder.Build<paddle::dialect::AddOp>(
      matmul_op3.out(), full_eleadd3_y_op.out());

  paddle::dialect::ReshapeOp reshape_op3 =
      builder.Build<paddle::dialect::ReshapeOp>(
          add_op3.out(), std::vector<int64_t>{1, 300, 8, 32});

  paddle::dialect::TransposeOp transpose_op3 =
      builder.Build<paddle::dialect::TransposeOp>(reshape_op3.out(),
                                                  std::vector<int>{0, 2, 1, 3});

  paddle::dialect::ScaleOp scale_op1 = builder.Build<paddle::dialect::ScaleOp>(
      transpose_op3.out(), 0.1767766922712326, 0.0, true);

  paddle::dialect::MatmulOp matmul_op4 =
      builder.Build<paddle::dialect::MatmulOp>(
          scale_op1.out(), transpose_op2.out(), false, true);

  paddle::dialect::SoftmaxOp softmax_op1 =
      builder.Build<paddle::dialect::SoftmaxOp>(matmul_op4.out(), -1);

  // tail
  paddle::dialect::MatmulOp matmul_op5 =
      builder.Build<paddle::dialect::MatmulOp>(
          softmax_op1.out(), transpose_op1.out(), false, false);

  paddle::dialect::TransposeOp transpose_op4 =
      builder.Build<paddle::dialect::TransposeOp>(matmul_op5.out(),
                                                  std::vector<int>{0, 2, 1, 3});

  paddle::dialect::ReshapeOp reshape_op4 =
      builder.Build<paddle::dialect::ReshapeOp>(
          transpose_op4.out(), std::vector<int64_t>{1, 300, 256});

  //   paddle::dialect::FullOp full_mat4_y_op =
  //       builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256,
  //       256},
  //                                              1.1,
  //                                              phi::DataType::FLOAT32,
  //                                              phi::CPUPlace());
  //   paddle::dialect::MatmulOp matmul_op6 =
  //       builder.Build<paddle::dialect::MatmulOp>(
  //           reshape_op4.out(), full_mat4_y_op.out(), false, false);

  //   paddle::dialect::FullOp full_eleadd4_y_op =
  //       builder.Build<paddle::dialect::FullOp>(std::vector<int64_t>{256},
  //                                              1.5,
  //                                              phi::DataType::FLOAT32,
  //                                              phi::CPUPlace());
  //   paddle::dialect::AddOp add_op4 = builder.Build<paddle::dialect::AddOp>(
  //       matmul_op6.out(), full_eleadd4_y_op.out());

  //   paddle::dialect::FullOp full_slice_axes_op =
  //       builder.Build<paddle::dialect::FullOp>(
  //           std::vector<int>{64}, 2, phi::DataType::INT64, phi::CPUPlace());
  //   paddle::dialect::FullOp full_slice_starts_op =
  //       builder.Build<paddle::dialect::FullOp>(
  //           std::vector<int>{64}, 2, phi::DataType::INT64, phi::CPUPlace());
  //   paddle::dialect::SliceOp slice_op1 =
  //   builder.Build<paddle::dialect::SliceOp>(
  //       add_op4.out(), full_slice_axes_op.out(), full_slice_starts_op);

  builder.Build<paddle::dialect::FetchOp>(reshape_op4.out(), "out", 0);
}

TEST(DrrTest, AttentionFuse) {
  ir::IrContext *ctx = ir::IrContext::Instance();
  ctx->GetOrRegisterDialect<paddle::dialect::PaddleDialect>();
  ir::Program program(ctx);
  ir::Builder builder = ir::Builder(ctx, program.block());
  BuildProgram(builder);
  program.Print(std::cout);
  //   EXPECT_EQ(program.block()->size(), 14u);

  //   ir::PassManager pm(ctx);
  //   pm.AddPass(std::make_unique<DrrPatternRewritePass>());
  //   pm.AddPass(ir::CreateDeadCodeEliminationPass());
  //   // pm.EnablePassTiming();
  //   pm.EnableIRPrinting();

  //   CHECK_EQ(pm.Run(&program), true);
  //   EXPECT_EQ(program.block()->size(), 7u);
}
