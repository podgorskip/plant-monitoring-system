FROM node:18 AS build-stage

WORKDIR /app

COPY package*.json ./

RUN npm install

COPY . .

RUN npm run build -- --output-path=dist/smart-pot-frontend --configuration=production

FROM nginx:alpine AS production-stage

WORKDIR /usr/share/nginx/html

RUN rm -rf ./*

COPY --from=build-stage /app/dist/smart-pot-frontend ./

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]