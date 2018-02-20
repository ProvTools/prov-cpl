
sudo yum install -y centos-release-scl

sudo yum install -y redhat-lsb-core devtoolset-7 boost-devel unixODBC-devel rpm-build rpmdevtools

sudo yum install -y pcre-devel python-devel

wget http://prdownloads.sourceforge.net/swig/swig-3.0.12.tar.gz

tar xvfz swig-3.0.12.tar.gz

cd swig-3.0.12

source /opt/rh/devtoolset-7/enable

./configure

make

sudo make install

cd ..

#Create directories used by rpmbuild:

mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

#Change to the directory with the spec file:

#Download the sources:

spectool -g /prov-cpl/dataverse-provenance.spec -C ~/rpmbuild/SOURCES

#Build the RPM:

rpmbuild -ba dataverse-provenance.spec

#Verify the files in the RPM you've built:

#rpm -qpl /home/vagrant/rpmbuild/RPMS/x86_64/dataverse-provenance-0.1-1.x86_64.rpm

#The output should include files like /usr/lib64/libcpl.so.

#If your task is to update the RPM, bump the version or release number in the spec and add a changelog entry to the end. 
#The resulting RPM is so small (~200KB) that we host it right in the guides at 
#doc/sphinx-guides/source/_static/installation/files/home/rpmbuild/rpmbuild/RPMS/x86_64. 
#From the Vagrant environment, you can copy over a new RPM like this:

#cp /home/vagrant/rpmbuild/RPMS/x86_64/dataverse-provenance-0*.rpm /dataverse/doc/sphinx-guides/source/_static/installation/files/home/rpmbuild/rpmbuild/RPMS/x86_64
