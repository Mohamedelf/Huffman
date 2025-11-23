# Utiliser une image Linux légère avec Node.js
FROM node:18-slim

# Installer les outils pour compiler le C (gcc, make)
RUN apt-get update && apt-get install -y gcc make build-essential

# Créer le dossier de l'application
WORKDIR /app

# Copier les fichiers de dépendances
COPY package*.json ./

# Installer les dépendances Node.js
RUN npm install

# Copier tout le reste du projet
COPY . .

# Compiler le programme C (utilise ton Makefile)
RUN make

# Créer le dossier pour les uploads s'il n'existe pas
RUN mkdir -p uploads

# Exposer le port (Render utilise souvent un port dynamique)
EXPOSE 8080

# Lancer le serveur
CMD ["node", "server.js"]