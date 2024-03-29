// TODO this is a Vuex example, remove


import shop from '../../api/shop'

// initial state
const state = () => ({
    all: []
})

// getters
const getters = {}

// actions
const actions = {
    async getAllProducts ({ commit }) {
        const products = await shop.getProducts()
        commit('setProducts', products)
    }
}

// mutations
const mutations = {
    setProducts (state, products) {
        state.all = products
    },

    incrementProductInventory (state, { id }) {
        const product = state.all.find(product => product.id === id)
        product.inventory++
    },

    decrementProductInventory (state, { id }) {
        const product = state.all.find(product => product.id === id)
        product.inventory--
    }
}

export default {
    namespaced: true,
    state,
    getters,
    actions,
    mutations
}