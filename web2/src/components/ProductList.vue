<template>
  <el-row>
    <el-col
        :span="12"
        v-for="product in products"
        :key="product.id"
    >
      <el-card>
        <span>{{ product.title }}</span>
        <span>{{ product.price | currency }}</span>
        <div class="bottom clearfix">
          <el-button type="info" :disabled="!product.inventory" @click='addProductToCart(product)'>Add to cart</el-button>
        </div>
      </el-card>
    </el-col>
  </el-row>
</template>

<script>
import {mapState, mapActions} from 'vuex'

export default {
  computed: mapState({
    products: state => state.products.all
  }),
  methods: mapActions('cart', [
    'addProductToCart'
  ]),
  created() {
    this.$store.dispatch('products/getAllProducts')
  }
}
</script>
