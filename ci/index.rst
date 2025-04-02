=============
CI/CD Tooling
=============

Debian/Ubuntu PPA Release
=========================

To prepare a new release ``debian/main`` and ``pristine-tar`` branches needs
to be updated and published on github.

1. Ensure the local branches ``debian/main`` and ``pristine-tar`` match the
   ones on github remote and the tag with the new version to be released is
   available
2. Checkout ``debian/main``.
3. ``./ci/docker-run.sh`` and check for clean workspace `git clean -dnx`,
   usually you have to delete at least `ci/sshconfig`.
4. Run
   ``GIT_AUTHOR_NAME="Your Name" EMAIL="your@mail.org" ./ci/create_debian_release.sh x.y.z``
5. Push the branches ``debian/main`` and ``pristine-tar`` to github. Not on
   *main* and not on *integration*!

Commit Message Linting
======================

The pipeline now contains a commit linting step which uses
`commitlint <https://github.com/conventional-changelog/commitlint>`_ using its
configuration for
`Conventional Commits <https://www.conventionalcommits.org/en/v1.0.0/>`_.

Commitlint is now part of the build docker container and we have linting
wrapper script which will lint all commits that would be merged from your
current branch to a given target branch.

For example:

.. code-block:: sh

    ci/docker-run.sh
    ci/lint-commits.sh origin/integration

will lint all commits from HEAD that would be merged to origin/integration with
a PR. Note that this may not work if your feature branch is not up to date with
the target branch.

Running the Linter Outside Docker
---------------------------------

In order to use the script above outside Docker, you have to install
commitlint.

To install globally on a recent Ubuntu/Debian, you could do (note that this
will install an updated node.js in /usr/local):

.. code-block:: sh

   # install repository npm
   sudo apt install npm
   # install node updater globally
   sudo npm install n -g
   # update node
   sudo n stable
   # install commitlint and conventional commit configuration
   sudo npm install @commitlint/cli -g
   sudo npm install @commitlint/config-conventional -g

This should take care of everything. If you're getting path errors nonetheless,
you may need to set the ``NODE_PATH`` environment variable to
``/usr/local/lib/node_modules``.

Alternatively, if a global installation is not preferred, you can look for
local installation instructions using ``npm``/``npx`` in the commitlint
documentation. You can set the ``COMMITLINT_CMD`` environment variable to
indicate how your local ``commitlint`` should be executed if it is not in path
or if you want to use a wrapper like ``npx``.

Using the Optional Git Hook
---------------------------

There is a ``commit-msg`` hook for linting your commit message in the
``.githooks/`` directory. If you want to use it, you must have installed the
prerequisites to run the ``ci/lint-commits.sh`` script outside of docker, as
described above.

To then activate the git hook, run the following command *from the repo root*:

.. code-block:: sh

   git config --local  core.hooksPath .githooks/

If you want to bypass the hook for e.g. a work-in-progress commit, just use
``git commit -n`` or ``git commit --no-verify``.

Note that the hook will not complain about 'fixup' commits, assuming that you
plan to squash them later. However, the pipeline step will still prevent
'fixup' commits from being merged.

README Table-of-Contents Generation
===================================

The Table-of-Contents in README.md is auto-generated using doctoc. The tool
is included in the build container or can be installed via ``npm`` similar
to commitlint above:

.. code-block:: sh

   sudo npm install doctoc -g

Then (or in docker) you can run:

.. code-block:: sh

   ci/readme-toc.sh

to re-generate the ToC if you have made any alterations or additions to the
sections. The script will return ``1`` if there are unstaged changes in
README.md. This is a feature so the CI can check if the ToC is currently
in need of an update. For development the return code can be ignored.

Code-Formatting
===============

The code formatter script ``ci/format-code.sh`` will use ``clang-format``,
``shfmt`` and ``yapf3`` with the correct settings to format C code, header
files, shell, and Python scripts within the project.

Optionally, the ``--check`` parameter does a dry run, reporting errors if any
files do not conform to formatting rules.

The ``clang-format``, ``shfmt`` and ``yapf3`` tools should be available in most
distros. Alternatively, they are available n the build container as well.

Script Linting
==============

The script ``ci/lint-scripts.sh`` will use the ``flake8`` tool to lint the
Python code in the project. Settings are aligned with the ``yapf3`` formatter
mentioned above.

An extension of the script with shellcheck is planned.

Github Pages
============

To update our github pages, you need to first build the Doxygen and Sphinx
doc. To get best results for github pages, use an Ubuntu 24.04 build docker:

.. code-block:: sh

   ci/docker-run.sh amd64 noble

Inside the container, run

.. code-block:: sh

   # first, some clean up
   rm -rf doc/api result/amd64/doc/sphinx
   # build crinit and doxygen HTML
   ci/build.sh
   # build sphinx doc (it will automatically grab the doxygen HTML as well)
   ci/build_doc.sh
   # package the result excluding unneeded files
   cd result/amd64/doc/sphinx/html
   tar --exclude='./objects.inv' --exclude='./.doctrees' \
           --exclude='./.buildinfo' -czf /base/gh-pages.tar.gz .

The resulting tar file can be used to update the ``gh-pages`` branch inside this
repo by creating a new branch from it, unpacking the tar file to the branch root
and creating a pull request.
