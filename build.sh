svn up
rev=$(svnversion)

sed s/_svn_revision_replace_/$rev/ -i gqsync.pro
make packages
svn revert gqsync.pro
