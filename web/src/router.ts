import Vue from "vue";
import Router from "vue-router";

Vue.use(Router);

export default new Router({
  routes: [
    {
      path: "/",
      name: "liftControls",
      component: () => import(/* webpackChunkName: "update" */ "./views/LiftControls.vue"),
      meta:
      {
        displayName: "Lift Controls",
      },
    },
    {
      path: "/update",
      name: "update",
      component: () => import(/* webpackChunkName: "update" */ "./views/Update.vue"),
      meta:
      {
        displayName: "Update",
      },
    },
    {
      path: "/debug",
      name: "debug",
      component: () => import(/* webpackChunkName: "update" */ "./views/Debug.vue"),
      meta:
      {
        displayName: "Debug",
      },
    },
    {
      path: "/about",
      name: "about",
      // route level code-splitting
      // this generates a separate chunk (about.[hash].js) for this route
      // which is lazy-loaded when the route is visited.
      component: () => import(/* webpackChunkName: "about" */ "./views/About.vue"),
      meta:
      {
        displayName: "About",
      },
    },
  ],
});
