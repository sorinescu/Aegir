<template>
  <el-menu theme="dark" :default-active="activeIndex" class="el-menu-demo" mode="horizontal" router>
    <el-menu-item index="1">Employbl Store</el-menu-item>
    <el-submenu index="2">
      <template slot="title">Dropdown Items</template>
      <el-menu-item index="2-1">item one</el-menu-item>
      <el-menu-item index="2-2">item two</el-menu-item>
      <el-menu-item index="2-3">item three</el-menu-item>
    </el-submenu>
    <el-menu-item index="/upload-recipe">Upload recipe</el-menu-item>
    <el-menu-item index="/weigh-ingredients">Weigh Ingredients</el-menu-item>
  </el-menu>
</template>

<script>
export default {
  data() {
    return {
      activeIndex: '1',
      activeIndex2: '1'
    };
  },
  methods: {
    handleSelect(key, keyPath) {
      console.log(key, keyPath);
    }
  }
}
</script>