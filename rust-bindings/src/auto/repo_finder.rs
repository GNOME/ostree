// This file was generated by gir (https://github.com/gtk-rs/gir)
// from gir-files
// DO NOT EDIT

use crate::{ffi};
#[cfg(feature = "v2018_6")]
#[cfg_attr(docsrs, doc(cfg(feature = "v2018_6")))]
use crate::{RepoFinderResult};
use glib::{prelude::*};
#[cfg(feature = "v2018_6")]
#[cfg_attr(docsrs, doc(cfg(feature = "v2018_6")))]
use glib::{translate::*};

glib::wrapper! {
    #[doc(alias = "OstreeRepoFinder")]
    pub struct RepoFinder(Interface<ffi::OstreeRepoFinder, ffi::OstreeRepoFinderInterface>);

    match fn {
        type_ => || ffi::ostree_repo_finder_get_type(),
    }
}

impl RepoFinder {
        pub const NONE: Option<&'static RepoFinder> = None;
    
}

pub trait RepoFinderExt: IsA<RepoFinder> + 'static {}

impl<O: IsA<RepoFinder>> RepoFinderExt for O {}
