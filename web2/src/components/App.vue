<template>
  <div id="app">
    <nav-bar></nav-bar>
    <router-view></router-view>
    <el-row :gutter="20">
      <el-col :span="16">
        <h1>Product List</h1>
        <product-list></product-list>
      </el-col>
      <el-col :span="8">
        <h1>Shopping Cart</h1>
        <shopping-cart></shopping-cart>
      </el-col>
    </el-row>

  </div>
</template>

<script>
import NavBar from "./NavBar"
import ProductList from './ProductList.vue'
import ShoppingCart from './ShoppingCart.vue'

export default {
  components: {NavBar, ProductList, ShoppingCart}
}
</script>

<style lang="scss">
// Import Font Awesome 5 Free
$fa-css-prefix: 'el-icon-fa';
$fa-font-path: '~@fortawesome/fontawesome-free/webfonts';

@import '~@fortawesome/fontawesome-free/scss/fontawesome.scss';
//@import '~@fortawesome/fontawesome-free/scss/regular.scss';
@import '~@fortawesome/fontawesome-free/scss/solid.scss';
//@import '~@fortawesome/fontawesome-free/scss/brands.scss';

// Override Element UI's icon font
.fas {
  font-family: 'Font Awesome 5 Free' !important;
}

.fab {
  font-family: 'Font Awesome 5 Brands' !important;
}
</style>