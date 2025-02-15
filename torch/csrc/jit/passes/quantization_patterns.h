#pragma once

#include <string>
#include <unordered_map>

namespace torch {
namespace jit {

std::unordered_map<std::string, std::string> quant_fusion_pattern_and_replacements() {

  std::string conv2d = R"(
graph(%a_quant, %packed_params, %r_scale, %r_zero_point, %r_dtype, %stride, %padding, %dilation, %groups):
        %a_dequant = aten::dequantize(%a_quant)
        %w_quant : Tensor, %b : Tensor? = quantized::conv_unpack(%packed_params)
        %w_dequant = aten::dequantize(%w_quant)
        %r = aten::conv2d(%a_dequant, %w_dequant, %b, %stride, %padding, %dilation, %groups)
        %r_quant = aten::quantize_per_tensor(%r, %r_scale, %r_zero_point, %r_dtype)
        return (%r_quant))";

  std::string quantized_conv2d = R"(
graph(%a_quant, %packed_params, %r_scale, %r_zero_point, %r_dtype, %stride, %padding, %dilation, %groups):
        %r_quant = quantized::conv2d(%a_quant, %packed_params, %stride, %padding, %dilation, %groups, %r_scale, %r_zero_point)
        %0 : int = prim::Constant[value=0]()
        %1 : int = prim::Constant[value=1]()
        %2 : int = prim::Constant[value=2]()
        %3 : int = prim::Constant[value=3]()
        %out_param : int[] = prim::ListConstruct(%0, %3, %1, %2)
        %r_perm = aten::permute(%r_quant, %out_param)
        return (%r_perm))";

  std::string addmm = R"(
graph(%packed_params, %a_quant, %r_scale, %r_zero_point, %r_dtype, %4):
        %a_dequant = aten::dequantize(%a_quant)
        %w_quant : Tensor, %b : Tensor? = quantized::linear_unpack(%packed_params)
        %w_dequant = aten::dequantize(%w_quant)
        %w_dequant_t = aten::t(%w_dequant)
        %r = aten::addmm(%b, %a_dequant, %w_dequant_t, %4, %4)
        %r_quant = aten::quantize_per_tensor(%r, %r_scale, %r_zero_point, %r_dtype)
        return (%r_quant))";

  std::string matmul_with_bias = R"(
graph(%packed_params, %a_quant, %r_scale, %r_zero_point, %r_dtype, %4):
        %a_dequant = aten::dequantize(%a_quant)
        %w_quant : Tensor, %b : Tensor? = quantized::linear_unpack(%packed_params)
        %w_dequant = aten::dequantize(%w_quant)
        %w_dequant_t = aten::t(%w_dequant)
        %output = aten::matmul(%a_dequant, %w_dequant_t)
        %r = aten::add_(%output, %b, %4)
        %r_quant = aten::quantize_per_tensor(%r, %r_scale, %r_zero_point, %r_dtype)
        return (%r_quant))";

  std::string quantized_linear = R"(
graph(%packed_params, %a_quant, %r_scale, %r_zero_point, %r_dtype, %4):
        %r = quantized::linear(%a_quant, %packed_params, %r_scale, %r_zero_point)
        return (%r))";

  std::string matmul_no_bias = R"(
graph(%packed_params, %a_quant, %r_scale, %r_zero_point, %r_dtype):
        %a_dequant = aten::dequantize(%a_quant)
        %w_quant : Tensor, %b : Tensor? = quantized::linear_unpack(%packed_params)
        %w_dequant = aten::dequantize(%w_quant)
        %w_dequant_t = aten::t(%w_dequant)
        %r = aten::matmul(%a_dequant, %w_dequant_t)
        %r_quant = aten::quantize_per_tensor(%r, %r_scale, %r_zero_point, %r_dtype)
        return (%r_quant))";

  std::string quantized_linear_no_bias = R"(
graph(%packed_params, %a_quant, %r_scale, %r_zero_point, %r_dtype):
        %r = quantized::linear(%a_quant, %packed_params, %r_scale, %r_zero_point)
        return (%r))";

  return {
    {conv2d, quantized_conv2d},
    {addmm, quantized_linear},
    {matmul_with_bias, quantized_linear},
    {matmul_no_bias, quantized_linear_no_bias}
  };

}

}} // torch::jit
