# Mac Media Compact — Tessera 0.2.5

Tuile expérimentale pour Tessera Screen Manager et un lecteur `media_player` Home Assistant, notamment MacMediaPlayer.

## Fonctions
- Pochette carrée à gauche, titre, artiste et commandes centrés dans la partie droite de la tuile 2×1.
- La pochette provient de Home Assistant via Tessera. Son rafraîchissement dépend du cache de Tessera et de l'adresse d'image fournie par Home Assistant.
- Un toucher sur la tuile, hors commandes, ouvre une vue agrandie ; le bouton Retour de Tessera revient au tableau de bord.
- Lorsqu'il n'y a aucune lecture, le contenu de la tuile est vide. Une piste en pause reste affichée.
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
La tuile utilise la source Tessera `cover:<entité>`, qui récupère l'image via Home Assistant. Depuis la version 0.2.4, la tuile redemande aussi l'image quelques secondes après un changement, car les informations et la pochette peuvent arriver à des moments différents. Cette méthode expérimentale utilise un détail interne de Tessera et doit être testée sur l'écran. Si l'adresse de l'image Home Assistant reste identique, le cache de Tessera Screen Manager peut encore conserver l'ancienne image.

## État de validation
Manifeste et traductions vérifiés ; compilation du firmware et essai sur le GUITION encore nécessaires.

## Limites
- Le rendu repose sur les polices et couleurs de Tessera, pas sur les polices iOS.
- Les boutons précédent/suivant ne fonctionnent que si MacMediaPlayer et macOS prennent ces commandes en charge.
- Les miniatures YouTube ne sont pas garanties.
