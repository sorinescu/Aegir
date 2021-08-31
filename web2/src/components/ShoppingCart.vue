<template>
  <div>
    <el-table
        :data="products"
        stripe
        style="width: 100%">
      <el-table-column
          prop="title"
          label="Item Name"></el-table-column>
      <el-table-column
          label="Price">
        <template slot-scope='scope'>
          {{ scope.row.price | currency }}
        </template>
      </el-table-column>
      <el-table-column
          prop="quantity"
          label="Quantity"></el-table-column>
      <el-table-column
          label="">
        <template slot-scope="scope">
          <el-button type="success" icon="fas el-icon-fa-plus" @click='addProductToCart(scope.row)' size="mini"></el-button>
          <el-button type="danger" icon="fas el-icon-fa-minus" @click='removeProductFromCart(scope.row.id)' size="mini"></el-button>
        </template>
      </el-table-column>
    </el-table>
    <p>Total: {{ total | currency }}</p>
    <p>
      <button :disabled="!products.length" @click="checkout(products)">Checkout</button>
    </p>
    <p v-show="checkoutStatus">Checkout {{ checkoutStatus }}.</p>
  </div>
</template>

<script>
import {mapActions, mapGetters, mapState} from 'vuex'

export default {
  computed: {
    ...mapState({
      checkoutStatus: state => state.cart.checkoutStatus
    }),
    ...mapGetters('cart', {
      products: 'cartProducts',
      total: 'cartTotalPrice'
    })
  },
  methods: {
    ...mapActions('cart', [
      'addProductToCart',
      'removeProductFromCart'
    ]),
    checkout(products) {
      this.$store.dispatch('cart/checkout', products)
    }
  }
}
</script>
