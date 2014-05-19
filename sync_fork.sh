#! /bin/sh

# fetch upstream
git fetch upstream

# merge

# check out our locallocal master branch
# Switched to branch 'master'
git checkout master

# Merge upstream/master into our own
git merge upstream/master

