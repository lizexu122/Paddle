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

#pragma once

#include "paddle/pir/core/builder.h"
#include "paddle/pir/core/builtin_type_interfaces.h"
#include "paddle/pir/core/ir_printer.h"
#include "paddle/pir/core/op_base.h"

namespace pir::dialect {

class IR_API SymbolicDim : public Op<SymbolicDim> {
 public:
  using Op::Op;
  static const char *name() { return "shape.symbolic_dim"; }

  static constexpr uint32_t attributes_num = 6;
  static const char *attributes_name[attributes_num];

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    const std::string &sym_name,
                    int64_t value = ShapedTypeInterface::kDynamic,
                    bool known_non_negative = false,
                    bool known_negative_one = false,
                    bool known_non_size_one = false,
                    bool known_non_size_zero = false);

  const std::string GetSymName();
  int64_t GetDimSize();

  bool GetKnownNonNegative();
  bool GetKnownNegativeOne();
  bool GetKnownNonSizeOne();
  bool GetKnownNonSizeZero();

  void SetSymName(const std::string &attr_value);
  void SetDimSize(int64_t attr_value);

  // Sets `known_non_negative` to the value of `flag`
  void UpdateKnownNonNegative(bool flag);

  // Sets `known_negative_one` to the value of `flag`
  void UpdateKnownNegativeOne(bool flag);

  // Sets `known_non_size_one` to the value of `flag`
  void UpdateKnownNonSizeOne(bool flag);

  // Sets `known_non_size_zero` to the value of `flag`
  void UpdateKnownNonSizeZero(bool flag);

  // Returns true if this SymbolicDim is not known at compile-time.
  bool IsDynamic();

  // Try to merge two SymbolicDim ops.
  bool Merge(SymbolicDim other);

  static const std::string GetSymbolicDimAttrName() {
    return "kSymbolicDimAttr";
  }

  void VerifySig() {}
};

class IR_API DimOp : public Op<DimOp> {
 public:
  using Op::Op;
  static const char *name() { return "shape.dim"; }

  static constexpr uint32_t attributes_num = 1;
  static const char *attributes_name[attributes_num];

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    const std::string &name);

  const std::string getName();
  void setName(std::string attrValue);
  OpResult out() { return result(0); }
  void VerifySig() {}
};

class IR_API TieProductEqualOp : public Op<TieProductEqualOp> {
 public:
  using Op::Op;
  static const char *name() { return "shape.tie_product_equal"; }

  static constexpr uint32_t attributes_num = 2;
  static const char *attributes_name[attributes_num];

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    int64_t lhs_len,
                    int64_t rhs_len,
                    const std::vector<Value> &inputs);
  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    const std::vector<Value> &lhs,
                    const std::vector<Value> &rhs);
  std::vector<Value> lhs();
  std::vector<Value> rhs();
  void VerifySig() {}
};

class IR_API TieShapeOp : public Op<TieShapeOp> {
 public:
  using Op::Op;
  static const char *name() { return "shape.tie_shape"; }

  static constexpr uint32_t attributes_num = 1;
  static const char *attributes_name[attributes_num];

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    pir::Value input);

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    Value input,
                    const std::vector<Value> &dims);
  Value value();
  std::vector<Value> dims();
  void VerifySig() {}
};

class IR_API FuncOp : public Op<FuncOp> {
 public:
  using Op::Op;
  static const char *name() { return "shape.func"; }

  static constexpr const char **attributes_name = nullptr;
  static constexpr uint32_t attributes_num = 0;

  static void Build(Builder &builder,              // NOLINT
                    OperationArgument &argument);  // NOLINT
  void Print(IrPrinter &printer);                  // NOLINT
  Block *block();
  void VerifySig() {}
};

class IR_API TensorDimOp : public Op<TensorDimOp> {
 public:
  using Op::Op;
  static const char *name() { return "shape.tensor_dim"; }

  static constexpr const char **attributes_name = nullptr;
  static constexpr uint32_t attributes_num = 0;

  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    Value source,
                    Value index);
  static void Build(Builder &builder,             // NOLINT
                    OperationArgument &argument,  // NOLINT
                    Value source,
                    int64_t index);
  Value index();
  Value source();
  OpResult out() { return result(0); }
  void VerifySig() {}
};

}  // namespace pir::dialect

IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::SymbolicDim);
IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::DimOp);
IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::TieProductEqualOp);
IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::TieShapeOp);
IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::FuncOp);
IR_EXPORT_DECLARE_EXPLICIT_TYPE_ID(pir::dialect::TensorDimOp);
