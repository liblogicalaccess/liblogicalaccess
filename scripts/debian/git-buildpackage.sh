#!/bin/bash

GIT_BRANCH=$BRANCH_NAME
CODENAME=`lsb_release -da  | grep Codename |  awk  '{print $2}'`
DISTRIB=`lsb_release -da  | grep Distributor |  awk  '{print $3}'`

if [ "$GIT_BRANCH" == "develop" ]; then

    # Just checking if Debian or not.
	if [ "$CODENAME" == "stretch" ]; then
		CODENAME="unstable"
	elif [ "$CODENAME" == "bionic" ]; then
		  CODENAME="bionic-backports"
	fi  
  
elif [ "$GIT_BRANCH" == "master" ]; then

	if [ "$CODENAME" == "stretch" ]; then
		CODENAME="stable"
	fi
	
fi

echo "GIT_BRANCH:"${BRANCH_NAME}
echo "GIT_COMMIT:"${GIT_COMMIT}
echo "CODENAME:"${CODENAME}
echo "DISTRIB:"${DISTRIB}

cd ~
git clone https://github.com/liblogicalaccess/liblogicalaccess-debian.git
cd liblogicalaccess-debian

git checkout -b upstream
git remote add liblogicalaccess https://github.com/liblogicalaccess/liblogicalaccess.git
git pull --no-edit --allow-unrelated-histories -X theirs  liblogicalaccess ${GIT_BRANCH}
git submodule init
git submodule update

VERSION=`cat pom.xml  | grep "<version>" -m1 | cut -d">" -f2 | cut -d"<" -f 1 |  cut -d"-" -f 1`.`date '+%Y%m%d'`
VERSION_DEBIAN=$VERSION~${CODENAME}
echo "VERSION=$VERSION"
echo "VERSION_DEBIAN=$VERSION_DEBIAN"

git tag upstream/$VERSION -f

git checkout master
git rebase upstream


#check if master and already have the hash dont push otherwise -> do
DEBFULLNAME=LIBLOGICALACCESS DEBEMAIL=support@liblogicalaccess.com  debchange -v $VERSION_DEBIAN --distribution $CODENAME "${GIT_BRANCH}-${GIT_COMMIT}"

export DEB_BUILD_OPTIONS="parallel=3"
gbp buildpackage --git-ignore-new --git-upstream-tree=upstream/$VERSION
