// TODO this is a Vuex example, remove


export default {
    namespaced: true,
    state: () => ({
        foo: 'bar'
    }),
    getters: {
        twoBars: state => state.foo.repeat(2)
    }
}