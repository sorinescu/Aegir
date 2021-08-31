import Vue from 'vue'
import { Button, Card, Col, Menu, MenuItem, Row, Submenu, Table, TableColumn } from 'element-ui'
import lang from 'element-ui/lib/locale/lang/en'
import locale from 'element-ui/lib/locale'
import App from './components/App.vue'
import store from './store'
import { currency } from './currency'
import { weight } from './weight'

locale.use(lang)

Vue.filter('currency', currency)
Vue.filter('weight', weight)

Vue.use(Button)
Vue.use(Card)
Vue.use(Col)
Vue.use(Menu)
Vue.use(MenuItem)
Vue.use(Row)
Vue.use(Submenu)
Vue.use(Table)
Vue.use(TableColumn)

new Vue({
    el: '#app',
    store,
    render: h => h(App)
})
