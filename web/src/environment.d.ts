declare global {
    namespace NodeJS {
        interface ProcessEnv {
            NODE_ENV: "development" | "dev-localhost" | "dev-tvlift" | "production";
            VUE_APP_BASEURI: string | undefined;
            VUE_APP_VERSION: string | undefined;
        }
    }
}

export {}