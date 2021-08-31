import Vue from 'vue'
import VueRouter from "vue-router"
import {Button, Card, Col, Menu, MenuItem, Row, Submenu, Table, TableColumn, Upload} from 'element-ui'
import lang from 'element-ui/lib/locale/lang/en'
import locale from 'element-ui/lib/locale'
import store from './store'
import {currency} from './currency'
import {weight} from './weight'
import WeighIngredients from "./components/WeighIngredients";
import UploadRecipe from "./components/UploadRecipe";
import App from './components/App.vue'

locale.use(lang)

Vue.filter('currency', currency)
Vue.filter('weight', weight)
Vue.use(VueRouter)

Vue.use(Button)
Vue.use(Card)
Vue.use(Col)
Vue.use(Menu)
Vue.use(MenuItem)
Vue.use(Row)
Vue.use(Submenu)
Vue.use(Table)
Vue.use(TableColumn)
Vue.use(Upload)

const router = new VueRouter({
    routes: [
        {path: '/upload-recipe', component: UploadRecipe},
        {path: '/weigh-ingredients', component: WeighIngredients}
    ]
})

new Vue({
    el: '#app',
    router,
    store,
    render: h => h(App)
})
