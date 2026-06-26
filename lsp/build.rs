fn main() {
    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-lib=static=cera");
    println!("cargo:rerun-if-changed=../libcera.a");
}
