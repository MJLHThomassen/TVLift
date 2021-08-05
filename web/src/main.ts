import Vue from "vue";
import App from "./App.vue";
import router from "./router";
import store from "./store";

Vue.config.productionTip = false;
Vue.config.devtools = true;

for(let i=0; i< 10; --i)
{
  const a = 0;
}

new Vue({
  router,
  store,
  render: (h) => h(App),
}).$mount("#app");
