# Byng

Byng is a search engine made with c++

# Deployment instructions
Byng deployment requires Vagrant ,Virtualbox et Ansible.It's also recommended to deploy Bing in an Unix environment.

# Cluster deployment
We have to get a special key to ensure the cluster consistency.
Go to https://discovery.etcd.io/new, an Url will be displayed. Copy it and replace KEY_HERE in cluster/user-data by generated url.

This stape is very important because without it,the cluster won't start.Once the key placed, the cluster can be started :

 cd cluster 
 vagrant up

In few minutes, 3 virtual machines will be created with CoreOS

# Master server deployment
The master server is an Ubuntu 14.04 LTS with Ansible.So, there's almost nothing to configure besides the DNS server of the 'host'  machine

- Back to the project racine byng-master
   vagrant up

The master server provision can take a certain time depending on the internet connexion quality and the server power

During this time,add the "172.17.8.254" address to your DNS servers or if you already have a local server,make sure that he "forwards" that he requests of .byng domain to that IP.

Note: if ever,the server provisioning gets an error, restart with "vagrant provision"

#Containers' launcehing
Now that all servers are ready ,it's time to launch the database.This command will launch 3 database conatainers and save them in the pool:

- Always in the folder
  vagrant ssh
- We are now in the master server
  ssh-add /vagrant.key
  fleectl ----tunnel 172.17.8.101:22 start /vagrant/cluster/by
  ng.db.*

- Creating the database:
  sudo -u postgres createuser byng -p 5433
  sudo -u postgres createdb byng -o byng -p 5433
- Database importation 
  sudo -u postgres psql -p 5433 byng < /vagrant/config/schema.sql

And here you are with a consistent database

#Let's launch the API now

- Always in the VM
  fleectl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.api*
# Then the web servers 
 fleectl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.web.*
# Then the crawler
  fleectl --tunnel 172.17.8.101:22 start /vagrant/cluster         byng.crawler.*

#And that's all
-----------------------------------------------------------------------
 If all gone right ,the master server should have compiled the different Byng elements and sent them to the Docker registry.The cluster should have received the launch instructions and is ready to work.

If your DNS have been correctly configured,you should be able to access to the search interface at http://search.byng address.



 
