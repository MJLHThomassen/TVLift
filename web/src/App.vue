<template>
    <div id="app">

        <!-- Global Header -->
        <header class="sticky">
            <router-link class="logo" to="/">TV Lift</router-link>

            <div  style="float:right;">
                <!-- Status -->
                <wifi-off-icon class="disconnected-icon" v-if="!statusService.isConnected"></wifi-off-icon>

                <!-- Menu Button -->
                <label for="drawer-control" class="button drawer-toggle persistent"></label>
            </div>

            <!-- Menu -->
            <input type="checkbox" id="drawer-control" class="drawer persistent">
            <nav>
                <h3>Menu</h3>
                <label for="drawer-control" class="drawer-close"></label>
                <!-- <div>
                <input style="width: 100%; margin: 0px;" placeholder="Search..." type="search" id="search-bar" oninput="search()">
                </div> -->
                <router-link v-for="route in $router.options.routes" :key="route.path" :to="route.path">{{ route.meta.displayName }}</router-link>
            </nav>
        </header>

        <!-- Main View -->
        <main>
            <template v-if="statusService.liftStatus === 'unknown'">
                <p>Loading...</p>
            </template>
            <template v-else>
                <router-view/>
            </template>
        </main>

        <!-- Global Footer -->
        <footer>
            <p>© 2020 Maarten Thomassen | <router-link to="/about">About</router-link></p>
        </footer>

    </div>
</template>

<script lang="ts">
import { Component, Vue, Provide } from "vue-property-decorator";

import "reflect-metadata";
import { container, DependencyContainer } from "tsyringe";

import Axios, { AxiosInstance } from "axios";
import { IStatusService } from "@/services/iStatusService";
import { IWebsocketService } from "@/services/iWebsocketService";
import { StatusService } from "@/services/statusService";
import { WebsocketService } from "@/services/websocketService";

import { WifiOffIcon } from "vue-feather-icons";

import "mini.css/dist/mini-dark.min.css";

//#region DI

// Register Constants
//const deviceUri: "http://tvlift.local/api/";
const baseUri: string = process.env.NODE_ENV === "development" ? "http://localhost:8081/api" : "api/";
container.registerInstance("baseUri", baseUri );

// Register 3rd Party Services
container.registerInstance(
    "AxiosInstance",
    Axios.create({
        baseURL: container.resolve<string>("baseUri"),
        timeout: 200
    })
);

// Register Services
container.registerSingleton(WebsocketService).registerType(IWebsocketService.name, WebsocketService);
container.registerSingleton(StatusService).registerType(IStatusService.name, StatusService);

//#endregion

@Component({
    name: "App",
    components: {
        WifiOffIcon,
    },
})
export default class App extends Vue
{
    @Provide() private axios = container.resolve<AxiosInstance>("AxiosInstance");

    @Provide() private websocketService = container.resolve<IWebsocketService>(WebsocketService);
    @Provide() private statusService = container.resolve<IStatusService>(StatusService);
}
</script>

<style lang="scss">

:root
{
    --link-color: #42b983;
    --border-color: grey;

    --info-color: rgb(13, 152, 194);
    --warning-color: #ffca28;
    --error-color: #b71c1c;

    --a-link-color: var(--link-color);
    --footer-link-color: var(--link-color);;

    --header-border-color: var(--border-color);
    --footer-border-color: var(--border-color);
    --card-border-color: var(--border-color);

    .card.warning
    {
        --card-back-color: var(--warning-color);
    }

    .card.error
    {
        --card-back-color: var(--error-color);
    }
}

html,
body
{
    height:100%;
    margin: 0;
}

div#app
{
    
    height: 100%;

    display: flex;
    flex-direction: column;
    flex-wrap: nowrap;
    justify-content: flex-start;
    align-items: stretch;

    header
    {
        flex-grow: 0;
        flex-shrink: 0;
        flex-basis: auto;

        .disconnected-icon
        {
            color: var(--error-color);
        }
    }

    main
    {
        flex-grow: 1;
        flex-shrink: 1;
        flex-basis: 0;
        
        overflow: hidden;
    }

    footer
    {
        flex-grow: 0;
        flex-shrink: 0;
        flex-basis: 0;

        text-align: center;
    }
}
</style>
