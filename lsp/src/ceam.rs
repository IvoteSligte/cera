use core::ffi::c_char;

#[repr(C)]
#[derive(Debug)]
pub struct CompileError {
    // One-based line number.
    pub line: usize,
    // Zero-based column number.
    pub column: usize,
    // Length of the symbol this error is about.
    pub length: usize,
    pub message: *mut c_char,
}

#[repr(C)]
#[derive(Debug)]
pub struct CompileErrors {
    pub data: *mut CompileError,
    pub length: usize,
}

impl Drop for CompileErrors {
    fn drop(&mut self) {
        unsafe { free_compile_errors(self) };
    }
}

unsafe extern "C" {
    pub fn diagnose(source: *const c_char) -> CompileErrors;
    pub fn free_compile_errors(errors: *mut CompileErrors);
}
