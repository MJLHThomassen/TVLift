declare global {
    namespace NodeJS {
        interface ProcessEnv {
            NODE_ENV: "development" | "dev-localhost" | "dev-tvlift" | "production";
            VUE_APP_BASEURI: string;
            VUE_APP_VERSION: string
        }
    }
}

export {}