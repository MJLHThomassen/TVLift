<template>
  <div class="update">
    <form>
      <fieldset class="col-sm-12 col-md-8 col-md-offset-2 col-lg-6 col-lg-offset-3">
        <legend>Update</legend>
          <div class="dropbox">
            <input id="firmware-file-input" type="file" @change="firmwareFileChanged"/>
            <p>
              Drag your file here to begin<br>or click to browse
            </p>
          </div>
          <button v-bind:disabled="!canUpdate" type="button" class="button primary" @click="update">Update!</button>
      </fieldset>
      <fieldset v-if="isUpdating || updateErrorMessage" class="col-sm-12 col-md-8 col-md-offset-2 col-lg-6 col-lg-offset-3">
        <legend>Status</legend>
        <div>
          <p>Updating: {{ updateProgress }} %</p>
          <progress :value="updateProgress * 10" max="1000" class="primary"></progress>
        </div>
        <div v-if="updateErrorMessage" class="card fluid error">
          {{ updateErrorMessage }}
        </div>
      </fieldset>
    </form>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from "vue-property-decorator";
import { UploadIcon } from "vue-feather-icons";
import axios, { AxiosError } from "axios";

import { sleep } from "@/services/sleep";

@Component({
  components: {
    UploadIcon,
  },
})
export default class LiftControls extends Vue
{

  public get canUpdate(): boolean
  {
    return this.selectedFirmwareFile != null && !this.isUpdating;
  }

  private selectedFirmwareFile: File | null = null;
  private isUpdating: boolean = false;
  private updateProgress: number = 0;
  private updateErrorMessage: string = "";

  private firmwareFileChanged(event: Event): void
  {
    const fileInput = event.target as HTMLInputElement;

    if (fileInput != null && fileInput.files != null)
    {
      this.selectedFirmwareFile = fileInput.files[0];
    }
  }

  private async update(): Promise<void>
  {
    if (this.selectedFirmwareFile == null)
    {
      return;
    }

    this.updateErrorMessage = "";
    this.isUpdating = true;

    const data = new FormData();
    data.set("firmware", this.selectedFirmwareFile, this.selectedFirmwareFile.name);

    try
    {
      const response = await axios.post(
        "upload/firmware",
        data,
        {
          onUploadProgress: (progressEvent: ProgressEvent) =>
          {
            this.updateProgress = Math.round(100 * progressEvent.loaded / progressEvent.total);
          },
        });

      console.log(response);
    }
    catch (e)
    {
      this.updateProgress = 0;

      if (this.isAxiosError(e))
      {
        console.log(e.message);
        this.updateErrorMessage = e.message;
      }
      else
      {
        console.log("Something went wrong: " + e);
        this.updateErrorMessage = e;
      }
    }

    this.isUpdating = false;
  }

  private isAxiosError(object: any): object is AxiosError
  {
    return object.isAxiosError;
  }
}
</script>

<style lang="scss">
.update
{
  text-align: center;
}

.dropbox
{
    border: 2px dashed var(--border-color); /* the dash box */
    color: dimgray;
    margin: 10px 10px;
    position: relative;

    text-align: left;

    p
    {
      text-align: center
    }
}

.dropbox input
{
    opacity: 0; // invisible but it"s there!
    
    position: absolute;
    width: 100%;
    height: 100%;

    cursor: pointer;
    
    // Make sure the entire area uses the custom cursor set above,
    // apply some tricks to remove the default button since that
    // triggers the default mouse cursor, even with opacity at 0
    font-size: 0; // Make the button as small as possible
    text-indent: -100px; // "Squish" the button away completely
}
</style>

