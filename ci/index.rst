CI/CD Tooling
=============

Debian/Ubuntu PPA Release
-------------------------

To prepare a new release `debian/main` and `pristine-tar` branches needs to
be updated and published on github.

1. Ensure the local branches `debian/main` and `pristine-tar` match the ones
   on github remote and the tag with the new version to be released is
   available
2. Checkout `debian/main`.
3. `./ci/docker-run.sh` and check for clean workspace `git clean -dnx`,
   usually you have to delete at least `ci/sshconfig`.
4. Run `GIT_AUTHOR_NAME="Your Name" EMAIL="your@mail.org" ./ci/create_debian_release.sh x.y.z` 
5. Push the branches `debian/main` and `pristine-tar` to github. Not on *main*
   and not on *integration*!
