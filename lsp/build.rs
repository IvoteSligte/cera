fn main() {
    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-lib=static=ceam");
    println!("cargo:rerun-if-changed=../libceam.a");
}
