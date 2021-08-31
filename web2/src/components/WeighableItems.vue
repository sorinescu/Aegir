<!--
An item that can be weighed.
Properties:
* id - an integer to identify the item
* label - the name of the item
* target_weight - the quantity we need for the recipe
* current_weight - the current weight
-->
<template>
  <el-row>
    <el-col
        :span="12"
        v-for="item in items"
        :key="item.id"
    >
      <el-card class="box-card">
        <el-descriptions title="{{ item.label }}" direction="vertical" :column="2">
          <template slot="extra">
            <el-button type="primary" size="small">Select</el-button>
          </template>
          <el-descriptions-item label="Total needed">{{ item.target_weight | weight }}</el-descriptions-item>
          <el-descriptions-item label="Remaining">{{ item.target_weight - item.current_weight | weight }}</el-descriptions-item>
        </el-descriptions>
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
  ])
}
</script>
