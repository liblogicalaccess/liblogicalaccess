#!/bin/bash
## 
## We use the static-pages for hand-crafted HTML content.
## We want to keep history of this static-pages branch.

## The gh-pages contains the full website: hand-crafted HTML + generated
## documentation. We are not interrested in the history of this branch:
##    * The repo will get fat. Fast.
##    * Since the documentation is generated, we can generated previous
##      version of the documentation manually.

set -e
set -x

REPO_DIRECTORY=/home/xaqq/Documents/liblogicalaccess
if [ ! -d "$REPO_DIRECTORY" ]; then
    echo "Cannot find repository directory."
    exit -1
fi
cd $REPO_DIRECTORY

BRANCH=develop

## Generate documentation tarball from the current tree.
git checkout $BRANCH
cd documentations
doxygen && cp resources/doxy-boot.js doc/html && tar cvf /tmp/lla_${BRANCH}_doc.tar -C doc/html .
cd ..

## Create a new gh-pages from the static-pages tree. 
git checkout static-pages
git branch -D gh-pages || true
git checkout -b gh-pages

## Extract the previous tarball.
mkdir -p doc/${BRANCH}/html/ doc/${BRANCH}/resources
tar xvf /tmp/lla_${BRANCH}_doc.tar -C doc/$BRANCH/html/
git add doc/$BRANCH
git commit -m "Adding generated documentation"
git push origin gh-pages -f

echo "Success!"
