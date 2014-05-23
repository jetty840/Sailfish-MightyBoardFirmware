#! /bin/sh

# fetch upstream
git fetch upstream

# merge

# check out our locallocal master branch
# Switched to branch 'master'
git checkout master

# Merge upstream/master into our own
git merge upstream/master

# Grabs online updates and merges them with your local work
git pull origin master

# Push local work with online repository
git push origin master
