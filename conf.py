# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'crinit'
copyright = '2023, emlix GmbH/Elektrobit Automotive GmbH'
author = 'anton.hillebrand@emlix.com'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'myst_parser',
    'sphinx.ext.autodoc',
    'sphinx.ext.doctest',
    'sphinxcontrib.programoutput',
    'sphinx_favicon',

    # copy button on code blocks in HTML doc
    'sphinx_copybutton',

    # for code documentation & referencing
    'sphinx.ext.viewcode',
    'sphinx.ext.napoleon',
    'sphinx_c_autodoc',
    'sphinx_c_autodoc.napoleon',
    'sphinx_c_autodoc.viewcode',
    'sphinx.ext.githubpages',
]

templates_path = ['doc/_templates']
exclude_patterns = ['deps/*', 'packaging/**', "build/**/README.md", "build/amd64/debbuild/**", "result/**", "build/amd64/doc/doxygen/doc/adr/**", "build/amd64/doc/sphinx/**", "build/amd64-Debug/**"]
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

language = 'en'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['images', 'build/amd64/doc/doxygen/doc/']
html_logo = 'images/crinit-logo.png'

favicons = [
    "favicon_16x16.png",
    "favicon_32x32.png",
]

master_doc = "index"

# c-autodoc
c_autodoc_roots = [
    './src/',
    './inc/',
  ]

set_type_checking_flag = True
myst_footnote_transition = False

