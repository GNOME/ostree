// This file was generated by gir (https://github.com/gtk-rs/gir)
// from gir-files
// DO NOT EDIT

use crate::{ffi};
use glib::{prelude::*,translate::*};

glib::wrapper! {
    #[doc(alias = "OstreeBootconfigParser")]
    pub struct BootconfigParser(Object<ffi::OstreeBootconfigParser>);

    match fn {
        type_ => || ffi::ostree_bootconfig_parser_get_type(),
    }
}

impl BootconfigParser {
    #[doc(alias = "ostree_bootconfig_parser_new")]
    pub fn new() -> BootconfigParser {
        unsafe {
            from_glib_full(ffi::ostree_bootconfig_parser_new())
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_clone")]
#[must_use]
    pub fn clone(&self) -> BootconfigParser {
        unsafe {
            from_glib_full(ffi::ostree_bootconfig_parser_clone(self.to_glib_none().0))
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_get")]
    pub fn get(&self, key: &str) -> Option<glib::GString> {
        unsafe {
            from_glib_none(ffi::ostree_bootconfig_parser_get(self.to_glib_none().0, key.to_glib_none().0))
        }
    }

    #[cfg(feature = "v2020_7")]
    #[cfg_attr(docsrs, doc(cfg(feature = "v2020_7")))]
    #[doc(alias = "ostree_bootconfig_parser_get_overlay_initrds")]
    #[doc(alias = "get_overlay_initrds")]
    pub fn overlay_initrds(&self) -> Vec<glib::GString> {
        unsafe {
            FromGlibPtrContainer::from_glib_none(ffi::ostree_bootconfig_parser_get_overlay_initrds(self.to_glib_none().0))
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_parse")]
    pub fn parse(&self, path: &impl IsA<gio::File>, cancellable: Option<&impl IsA<gio::Cancellable>>) -> Result<(), glib::Error> {
        unsafe {
            let mut error = std::ptr::null_mut();
            let is_ok = ffi::ostree_bootconfig_parser_parse(self.to_glib_none().0, path.as_ref().to_glib_none().0, cancellable.map(|p| p.as_ref()).to_glib_none().0, &mut error);
            debug_assert_eq!(is_ok == glib::ffi::GFALSE, !error.is_null());
            if error.is_null() { Ok(()) } else { Err(from_glib_full(error)) }
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_parse_at")]
    pub fn parse_at(&self, dfd: i32, path: &str, cancellable: Option<&impl IsA<gio::Cancellable>>) -> Result<(), glib::Error> {
        unsafe {
            let mut error = std::ptr::null_mut();
            let is_ok = ffi::ostree_bootconfig_parser_parse_at(self.to_glib_none().0, dfd, path.to_glib_none().0, cancellable.map(|p| p.as_ref()).to_glib_none().0, &mut error);
            debug_assert_eq!(is_ok == glib::ffi::GFALSE, !error.is_null());
            if error.is_null() { Ok(()) } else { Err(from_glib_full(error)) }
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_set")]
    pub fn set(&self, key: &str, value: &str) {
        unsafe {
            ffi::ostree_bootconfig_parser_set(self.to_glib_none().0, key.to_glib_none().0, value.to_glib_none().0);
        }
    }

    #[cfg(feature = "v2020_7")]
    #[cfg_attr(docsrs, doc(cfg(feature = "v2020_7")))]
    #[doc(alias = "ostree_bootconfig_parser_set_overlay_initrds")]
    pub fn set_overlay_initrds(&self, initrds: &[&str]) {
        unsafe {
            ffi::ostree_bootconfig_parser_set_overlay_initrds(self.to_glib_none().0, initrds.to_glib_none().0);
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_write")]
    pub fn write(&self, output: &impl IsA<gio::File>, cancellable: Option<&impl IsA<gio::Cancellable>>) -> Result<(), glib::Error> {
        unsafe {
            let mut error = std::ptr::null_mut();
            let is_ok = ffi::ostree_bootconfig_parser_write(self.to_glib_none().0, output.as_ref().to_glib_none().0, cancellable.map(|p| p.as_ref()).to_glib_none().0, &mut error);
            debug_assert_eq!(is_ok == glib::ffi::GFALSE, !error.is_null());
            if error.is_null() { Ok(()) } else { Err(from_glib_full(error)) }
        }
    }

    #[doc(alias = "ostree_bootconfig_parser_write_at")]
    pub fn write_at(&self, dfd: i32, path: &str, cancellable: Option<&impl IsA<gio::Cancellable>>) -> Result<(), glib::Error> {
        unsafe {
            let mut error = std::ptr::null_mut();
            let is_ok = ffi::ostree_bootconfig_parser_write_at(self.to_glib_none().0, dfd, path.to_glib_none().0, cancellable.map(|p| p.as_ref()).to_glib_none().0, &mut error);
            debug_assert_eq!(is_ok == glib::ffi::GFALSE, !error.is_null());
            if error.is_null() { Ok(()) } else { Err(from_glib_full(error)) }
        }
    }
}

impl Default for BootconfigParser {
                     fn default() -> Self {
                         Self::new()
                     }
                 }
