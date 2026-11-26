# Mac Media Compact — Tessera 0.2.1

Tuile expérimentale pour Tessera Screen Manager et un lecteur `media_player` Home Assistant, notamment MacMediaPlayer.

## Fonctions
- Pochette carrée à gauche, titre, artiste et commandes centrés dans la partie droite de la tuile 2×1.
- La pochette provient de Home Assistant via Tessera. Son rafraîchissement dépend du cache de Tessera et de l'adresse d'image fournie par Home Assistant.
- Un toucher sur la pochette ouvre une vue agrandie ; le bouton Retour de Tessera revient immédiatement au tableau de bord.
- Pas de barre de volume ni de pochette étirée en arrière-plan.
- Commandes prévues : piste précédente, lecture/pause, piste suivante.
- L'illustration dépend de `entity_picture` exposée à Home Assistant.
- YouTube et Apple Music affichent les métadonnées disponibles dans l'entité Mac.

## Installation
Il faut publier **le contenu du dossier** à la racine d'un dépôt GitHub public, puis dans Tessera → Plugins → « Ajouter avec un lien » renseigner l'adresse de ce dépôt et affecter le plugin à l'écran Bureau. Tessera devra compiler le firmware.

Après installation : Tessera → Bureau → Layout → Plugins → Mac Media Compact → sélectionner l'entité `media_player` du Mac → utiliser une taille 2×1 ou 2×2 → Save & send.

## Mise à jour sans toucher aux autres tuiles
1. Dans Tessera → Plugins, ouvre **Mac Media Compact** puis lance **Update**. Tessera recompile uniquement le firmware de l'écran auquel ce plugin est associé ; le layout et les autres tuiles restent en place.
2. Quand l'écran est de nouveau en ligne, ouvre la tuile existante puis **Save & send** une fois, afin que Tessera transmette les nouveaux attributs de pochette.
3. Lance deux morceaux différents. La pochette doit changer avec le titre ; touche-la pour ouvrir l'affichage agrandi, puis utilise le bouton Retour en haut à gauche.

## Diagnostic de la pochette
La tuile utilise la source Tessera `cover:<entité>`, qui récupère l'image via Home Assistant. La version 0.2.0 y ajoutait un paramètre que Tessera refuse : la version 0.2.1 le retire. Tessera Screen Manager conserve toutefois sa propre copie tant que l'adresse de l'image Home Assistant ne change pas : le rafraîchissement dans ce cas reste à résoudre et à tester.

## État de validation
Manifeste et traductions vérifiés ; compilation du firmware et essai sur le GUITION encore nécessaires.

## Limites
- Le rendu repose sur les polices et couleurs de Tessera, pas sur les polices iOS.
- Les boutons précédent/suivant ne fonctionnent que si MacMediaPlayer et macOS prennent ces commandes en charge.
- Les miniatures YouTube ne sont pas garanties.
