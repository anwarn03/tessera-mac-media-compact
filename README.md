# Mac Media Compact — prototype Tessera 0.1.0

Tuile expérimentale pour Tessera Screen Manager et un lecteur `media_player` Home Assistant, notamment MacMediaPlayer.

## Fonctions
- Pochette carrée à gauche, titre et artiste à droite, commandes en bas.
- Pas de barre de volume ni de pochette étirée en arrière-plan.
- Commandes prévues : piste précédente, lecture/pause, piste suivante.
- L'illustration dépend de `entity_picture` exposée à Home Assistant.
- YouTube et Apple Music affichent les métadonnées disponibles dans l'entité Mac.

## Installation
Il faut publier **le contenu du dossier** à la racine d'un dépôt GitHub public, puis dans Tessera → Plugins → « Ajouter avec un lien » renseigner l'adresse de ce dépôt et affecter le plugin à l'écran Bureau. Tessera devra compiler le firmware.

Après installation : Tessera → Bureau → Layout → Plugins → Mac Media Compact → sélectionner l'entité `media_player` du Mac → utiliser une taille 2×1 ou 2×2 → Save & send.

## État de validation
**PROTOTYPE NON COMPILÉ ET NON TESTÉ SUR LE GUITION.** La structure suit le template officiel et les API documentées de Tessera, mais le projet n'a pas été compilé contre le firmware 0.64/0.66. Une correction peut être nécessaire à la première compilation. N'installe pas ce prototype sur ton écran principal avant d'avoir une sauvegarde et de comprendre comment revenir à l'ancien firmware.

## Limites
- Le rendu repose sur les polices et couleurs de Tessera, pas sur les polices iOS.
- Les boutons précédent/suivant ne fonctionnent que si MacMediaPlayer et macOS prennent ces commandes en charge.
- Les miniatures YouTube ne sont pas garanties.
