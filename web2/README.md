# aegir

## Author's notes
Add ElementUI support https://github.com/ElementUI/vue-cli-plugin-element
```
vue create aegir
mv aegir web2
cd web2
vue add element
```

Load Element component on demand (smaller build) https://element.eleme.io/#/en-US/component/quickstart
```
npm install babel-plugin-component -D
```

Use FontAwesome icons instead of ElementUI
* https://stackoverflow.com/questions/54617007/how-to-use-font-awesome-5-icons-in-element-ui
* https://cli.vuejs.org/guide/css.html#referencing-assets

```
# Superseded by the steps below (subset of icons)
# npm install --save @fortawesome/fontawesome-free
npm install -D sass-loader@^10.1.1 sass
```

Use only a subset of the FontAwesome icons https://github.com/omacranger/fontawesome-subset
```
npm install --save-dev fontawesome-subset 
npm install --save-dev @fortawesome/fontawesome-free
```

## Project setup
```
npm install
```

### Compiles and hot-reloads for development
```
npm run serve
```

### Compiles and minifies for production
```
npm run build
```

### Lints and fixes files
```
npm run lint
```

### Customize configuration
See [Configuration Reference](https://cli.vuejs.org/config/).
