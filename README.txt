==============================================================================

		Mode d'emploi TP Systèmes Répartis

==============================================================================
			24 janvier 2014
Auteurs :
    
    Jean-Baptiste CADOT
    Xavier LOURDIN

Filière : 3F1

---------------------- Compilation des programmes -----------------------------

Pour la partie serveur :

    > gcc serveur.c entete.c -o serveur

Pour la partie client :

    > gcc client.c entete.c -o client


----------------------- Exécution des programmes ------------------------------

On lance tout d'abord le serveur :
    
    > ./serveur

Puis on va lancer les clients 1 par 1 :

    > ./client 1664 etud

(ici etud étant le nom d'hôte où le serveur s'exécute et 1664 est une célèbre 
marque de biè... euh le port de communication)

Ainsi les clients peuvent se connecter de n'importe où mais ils doivent connaître 
le nom d'hôte du serveur.

/!\ Il faut qu'un 4eme client se connecte pour que le serveur envoie aux clients 
la liste de tous les autres.

(En effet nous n'avons pas fait de thread et il faut donc une dernière connexion 
pour sortir du svc_run())

Ensuite on appuie sur la touche 'e' pour demander d'entrer en Section Critique (SC)
et il faut appuyer sur ENTER pour valider la saisie.

On peut aussi quitter le programme par le biais des saisies suivantes :
    -q, Q
    -exit
    -quit
