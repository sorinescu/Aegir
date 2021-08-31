<template>
  <el-upload
      class="upload-recipe"
      drag
      action="https://jsonplaceholder.typicode.com/posts/"
      accept="text/xml"
      :before-upload="beforeRecipeUpload"
      :on-success="handleRecipeUploadSuccess">
    <i class="el-icon-upload"></i>
    <div class="el-upload__text">Drop file here or <em>click to upload</em></div>
  </el-upload>
</template>

<script>
import {parseBeerXML} from "@/recipe";

export default {
  data() {
    return {
      recipeURL: ''
    };
  },
  methods: {
    handleRecipeUploadSuccess(res, file) {
      this.recipeURL = URL.createObjectURL(file.raw);
    },
    beforeRecipeUpload(file) {
      const isXML = file.type === 'text/xml';
      if (!isXML) {
        this.$message.error('recipe must be in BeerXML format!');
      }
      file.text().then(xml => {
        console.log(xml)
        const recipe = parseBeerXML(xml)
        console.log(recipe)
      })
      return isXML;
    }
  }
}</script>

<style scoped>

</style>