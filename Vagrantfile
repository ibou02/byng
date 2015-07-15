# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "trusty"
  config.vm.network "private_network", ip: "172.17.8.254"

  config.vm.provision "ansible" do |ansible|
    ansible.sudo = true
    ansible.extra_vars = { ansible_ssh_user: 'vagrant' }
    ansible.inventory_path = "inventory.py"
    ansible.playbook = "playbook.yml"
  end
end
