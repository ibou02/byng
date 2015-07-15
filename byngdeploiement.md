# Byng

Byng est un moteur de recherche développé en C++.

## Instructions de déploiement

Le déploiement de Byng nécessite [Vagrant](http://www.vagrantup.com), [Virtualbox](https://www.virtualbox.org) et [Ansible](http://ansible.com). Il est également recommandé de déployer Byng dans un environnement Unix. Aucun tests sous Windows n'a été fait.

Le déploiement de Byng consiste en un serveur maître avec DNS, Nginx, Docker, Etcd, etc... et un cluster CoreOS contenant l'API, le Crawler et l'interface web.

### Déploiement du cluster

Il nous faut récupérer une clé spéciale pour assurer la consistance du cluster. Accédez à [https://discovery.etcd.io/new](https://discovery.etcd.io/new), une url sera affichée. Copiez là et remplacez `KEY_HERE` dans `cluster/user-data` par l'url générée.

Cette étape est importante car sans elle le cluster ne pourra démarrer.

Une fois la clé mise en place, démarrons le cluster :
    
    cd cluster
    vagrant up
    
Voilà. Dans quelques minutes, 3 machines virtuelles seront crées avec le système d'exploitation CoreOS.

### Déploiement du serveur maître

Le serveur maître est un serveur Ubuntu 14.04 LTS provisionné par Ansible. Il n'y a ainsi pratiquement rien à configurer à part le serveur DNS de la machine "host".

    # De retour à la racine du projet byng-master
    vagrant up

La provision du serveur maître peut prendre un certain temps selon la qualité de la connexion internet et la puissance du serveur.

Pendant ce temps, ajoutez l'adresse "172.17.8.254" à vos serveurs DNS ou bien si vous avez déjà un serveur local, assurez-vous qu'il "forward" les requêtes du domaine .byng à cette IP.

Note: si jamais le provisionnement du serveur s'arrête sur une erreur, n'hésitez pas à le relancer avec `vagrant provision`.

### Lancement des containers

Maintenant que tous les serveurs sont prêts, il est temps de lancer la base de donnée. Cette commande va lancer 3 containers de base de donnée et les enregistrer dans la pool :
 
    # Toujours dans ce dossier
    vagrant ssh
    # Nous sommes maintenant dans le serveur maître
    ssh-add /vagrant.key
    fleetctl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.db.*
    
Création de la base de données :

    sudo -u postgres createuser byng -p 5433
    sudo -u postgres createdb byng -O byng -p 5433
    # Importation de la base de données
    sudo -u postgres psql -p 5433 byng < /vagrant/config/schema.sql
    
Et nous voilà avec une base de données consistante.

Lançons maintenant l'API :

    # Toujours dans la vm
    fleetctl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.api.*
    # Puis les serveurs webs
    fleetctl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.web.*
    # Puis les crawlers
    fleetctl --tunnel 172.17.8.101:22 start /vagrant/cluster/byng.crawler.*
    

## Et c'est tout

Si tout s'est bien déroulé, le serveur maître a dû compiler les différents éléments de Byng et les envoyer sur le registry Docker. Le cluster a également dû recevoir les instructions de lancement et est prêt à fonctionner.

Si vos dns ont été correctement configurés, vous devriez pouvoir accéder à l'interface de recherche à l'adresse [http://search.byng](http://search.byng).


