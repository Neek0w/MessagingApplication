Sujet 4 : Application de matchmaking avec messagerie instantanée

## Objectif et description :

L’objectif de ce projet est de développer une application de matchmaking permettant de former des groupes de trois utilisateurs basés sur des paramètres prédéfinis tels que le sexe et l'âge. Une fois connectés et authentifiés, les utilisateurs rempliront un formulaire lors de leur première connexion, indiquant leurs informations personnelles pertinentes pour le matchmaking. Le serveur central se chargera de former des groupes de discussion en fonction de ces paramètres. Une fois les groupes créés, une messagerie instantanée sera mise à disposition pour permettre aux membres du groupe de communiquer en temps réel.

## Les objectifs principaux de ce projet sont les suivants :

- [x] Permettre à plusieurs utilisateurs de se connecter à un serveur, de s’authentifier et de remplir un formulaire de première connexion avec des critères tels que le sexe et l'âge.

  - [x] Plusieurs utilisateurs se connectent au serveur
  - [x] Authentification (login)
    - [x] unique
  - [x] Formulaire

- [ ] Assurer la formation de groupes de trois utilisateurs selon les critères définis dans le formulaire.

  - [x] Formation des groupes
  - [x] Selon critères stricts (même âge, même sexe)
  - [ ] Selon critères laxistes
    - [x] Age <= 10 ans
    - [ ] Sexe diff

- [ ] Offrir un service de messagerie instantanée en temps réel au sein de ces groupes, facilitant la communication.

  - [x] Messagerie instantanée
  - [ ] Persistente (Stocker données utilisateur)
  - [x] Possibilité de quitter le groupe (/quit)
  - [ ] Chiffrer
  - [ ] Pouvoir se connecter groupe grâce un ID (/connect 1234)
  - [ ] Suppression du groupe (0 personne)
  - [ ] Rechercher groupe pour lancer le matchmaking(/search)
  - [ ] Création de Log

- [ ] GUI
  - [ ] Menu
  - [ ] /help

- [] Rédaction rapport

- [] Tests unitaire

## Exigences techniques :

- [x] Gestion et authentification des utilisateurs.
- [ ] Création de groupes de matchmaking basés sur les paramètres du formulaire (sexe, âge, etc.).
- [ ] Messagerie en temps réel pour les groupes formés.
