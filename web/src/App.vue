<template>
  <div id="app">
    
    <!-- Global Header -->
    <header class="sticky">
      <router-link class="logo" to="/">TV Lift</router-link>
      <label for="drawer-control" class="button drawer-toggle persistent" style="float:right;"></label>
    </header>

    <div class="container">
      <div class="row">
        
        <!-- Menu -->
        <input type="checkbox" id="drawer-control" class="drawer persistent">
        <nav>
          <h3>Menu</h3>
          <label for="drawer-control" class="drawer-close"></label>
          <!-- <div>
            <input style="width: 100%; margin: 0px;" placeholder="Search..." type="search" id="search-bar" oninput="search()">
          </div> -->
          <router-link to="/">Lift Controls</router-link>
          <router-link to="/about">About</router-link>
        </nav>
        
        <!-- Main View -->
        <main class="col-sm">
          <router-view/>
        </main>
      </div>
    </div>

    <!-- Global Footer -->
    <footer>
      <p>© 2019 Maarten Thomassen | <router-link to="/about">About</router-link></p>
    </footer>

  </div>
</template>

<script lang="ts">
import { Component, Vue, Ref } from 'vue-property-decorator';
import "mini.css/dist/mini-dark.min.css";

@Component({})
export default class App extends Vue
{
  public constructor()
  {
    super();
  }
}
</script>

<style lang="scss">

:root
{
  --link-color: #42b983;
  --border-color: grey;

  --waning-color: #ffca28;
  --error-color: #b71c1c;

  --a-link-color: var(--link-color);
  --footer-link-color: var(--link-color);;

  --header-border-color: var(--border-color);
  --footer-border-color: var(--border-color);
  --card-border-color: var(--border-color);
}

footer
{
  text-align: center;
}
</style>
