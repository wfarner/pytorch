#include <ATen/ATen.h>
#include <ATen/native/Resize.h>
#include <ATen/native/ResizeCommon.h>
#include <c10/core/TensorOptions.h>

namespace at { namespace native {

Tensor& resize_cpu_(Tensor& self, IntArrayRef size) {
#ifdef BUILD_NAMEDTENSOR
  if (self.has_names()) {
    return resize_named_tensor_(self, size);
  }
#endif
  auto* self_ = self.unsafeGetTensorImpl();
  resize_impl_cpu_(self_, size, /*strides=*/c10::nullopt);
  self_->maybe_zero_dim(size.size() == 0);
  return self;
}

// Call the sparse implementation in SparseTensor.cpp directly.
// A dynamic dispatch here is NOT necessary, so I didn't put
// this function in native_functions.yaml
Tensor& resize_as_sparse_(Tensor& self, const Tensor& src);

Tensor& resize_as_(
    Tensor& self,
    const Tensor& the_template,
    c10::optional<MemoryFormat> optional_memory_format) {
  auto memory_format =
      optional_memory_format.value_or(MemoryFormat::Contiguous);
  if (self.is_sparse() && the_template.is_sparse()) {
    TORCH_CHECK(
        memory_format == MemoryFormat::Contiguous,
        "Unsupported memory format for sparse tensor resize_as_ ",
        memory_format);
    return native::resize_as_sparse_(self, the_template);
  }
  Tensor& result = self.resize_(the_template.sizes());
  if (memory_format == MemoryFormat::Preserve) {
    memory_format = the_template.suggest_memory_format();
  }
  self.unsafeGetTensorImpl()->empty_tensor_restride(memory_format);
#ifdef BUILD_NAMEDTENSOR
  namedinference::propagate_names(result, the_template);
#endif
  return result;
}

} // namespace native
} // namespace at
