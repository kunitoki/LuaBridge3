# Configuration file for the Sphinx documentation builder.
#
# This builds the LuaBridge3 documentation set. Sources are authored in MyST
# Markdown so they render both on GitHub and through Sphinx with the Shibuya
# theme.
#
# Full list of options: https://www.sphinx-doc.org/en/master/usage/configuration.html

import datetime

project = "LuaBridge3"
copyright = f"{datetime.datetime.now().year}, kunitoki@gmail.com"
author = "kunitoki@gmail.com"
github = "https://github.com/kunitoki/LuaBridge3"

# -- General configuration ---------------------------------------------------

extensions = [
    "myst_parser",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.intersphinx",
]

myst_enable_extensions = [
    "colon_fence",     # ::: fenced directives, GitHub-friendly
    "deflist",
    "fieldlist",
    "linkify",
    "substitution",
    "tasklist",
]

# Auto-generate anchors for headings so cross-file links stay stable.
myst_heading_anchors = 3

autosectionlabel_prefix_document = True
source_suffix = {
    ".md": "markdown",
}

# The root document of the documentation tree.
root_doc = "index"

exclude_patterns = [
    "_build",
    "Thumbs.db",
    ".DS_Store",
    "requirements.txt",
]

# -- Options for HTML output -------------------------------------------------

html_theme = "shibuya"
html_title = "LuaBridge3 Documentation"
html_static_path = ["_static"]

html_css_files = [
    "shibuya-override.css",
    "rtd-flyout.css",
]

html_theme_options = {
    "accent_color": "blue",
    "color_mode": "auto",
    "github_url": github,
    "nav_socials": ["github"],
    "light_logo": "_static/logo-light.png",
    "dark_logo": "_static/logo-dark.png",
    # "announcement": "The content of the announcement",
    "nav_links": [
        {
            "title": "Introduction",
            "url": "introduction/index",
        },
        {
            "title": "Accessing C++ from Lua",
            "url": "cpp-from-lua/index",
        },
        {
            "title": "Passing Objects",
            "url": "passing-objects/index",
        },
        {
            "title": "Accessing Lua from C++",
            "url": "lua-from-cpp/index",
        },
        {
            "title": "API Reference",
            "url": "api-reference/index",
        },
    ]
}
