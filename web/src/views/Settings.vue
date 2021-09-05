<template>
    <div class="settings">
        <form @submit="settingsSubmitted">
            <fieldset
                v-if="currentSettings != undefined"
                class="col-sm-12 col-md-8 col-md-offset-2 col-lg-6 col-lg-offset-3"
            >
                <legend>Settings</legend>
                <div class="input-group vertical fluid">
                    <label for="liftMinSpeed">Minimum Speed</label>
                    <input
                        type="number"
                        id="liftMinSpeed"
                        placeholder="Minimum Speed"
                        v-model.number="currentSettings.liftMinSpeed"
                    />
                </div>
                <div class="input-group vertical fluid">
                    <label for="liftMaxSpeed">Maximum Speed</label>
                    <input
                        type="number"
                        id="liftMaxSpeed"
                        placeholder="Maximum Speed"
                        v-model.number="currentSettings.liftMaxSpeed"
                    />
                </div>
                <div class="input-group vertical fluid">
                    <label for="liftDefaultSpeed">Default Speed</label>
                    <input
                        type="number"
                        id="liftDefaultSpeed"
                        placeholder="Default Speed"
                        v-model.number="currentSettings.liftDefaultSpeed"
                    />
                </div>
                <button type="submit" class="primary">Save</button>
            </fieldset>
            <div v-else>
                Loading settings...
                {{currentSettings}}
            </div>
        </form>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Inject } from "vue-property-decorator";
import { SettingsRepository, LiftSettingsMessage } from "@/repositories/settingsRepository";
import { IStatusService } from "@/services/iStatusService";

@Component({})
export default class Debug extends Vue
{
    private currentSettings: LiftSettingsMessage = {
        liftMinSpeed: 0,
        liftMaxSpeed: 0,
        liftDefaultSpeed: 0
    };

    @Inject()
    private readonly settingsRepository!: SettingsRepository;

    @Inject()
    private readonly statusService!: IStatusService;

    private async mounted(): Promise<void>
    {
        this.currentSettings = await this.settingsRepository.getSettings();
    }

    private async settingsSubmitted(): Promise<void>
    {
        await this.settingsRepository.postSettings(
            this.currentSettings.liftMinSpeed,
            this.currentSettings.liftMaxSpeed,
            this.currentSettings.liftDefaultSpeed);

        this.currentSettings = await this.settingsRepository.getSettings();
    }
}
</script>

<style lang="scss">
.settings
{

}
</style>