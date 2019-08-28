<template>
  <div class="update">
    <form>
      <fieldset class="col-sm-12 col-md-8 col-md-offset-2 col-lg-6 col-lg-offset-3">
        <legend>Update</legend>
          <div class="dropbox">
            <input type="file" id="update-file-input" @change="updateFileChanged"/>
            <p>
              Drag your file here to begin<br>or click to browse
            </p>
          </div>
          <button v-bind:disabled="!canUpdate" type="button" class="button primary" @click="update">Update!</button>
          <progress v-if="isUpdating" :value="uploadProgress" max="1000" class="primary"></progress>
      </fieldset>
    </form>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import { UploadIcon } from 'vue-feather-icons';
import axios, { AxiosError } from 'axios';

@Component({
  components: {
    UploadIcon,
  },
})
export default class LiftControls extends Vue
{

  public get canUpdate(): boolean
  {
    return this.selectedFile != null && !this.isUpdating;
  }

  public selectedFile: File | null = null;
  public isUpdating: boolean = false;
  private uploadProgress: number = 0;

  private updateFileChanged(event: Event): void
  {
    const fileInput = event.target as HTMLInputElement;

    if (fileInput != null && fileInput.files != null)
    {
      this.selectedFile = fileInput.files[0];
    }
  }

  private async update(): Promise<void>
  {
    if (this.selectedFile == null)
    {
      return;
    }

    this.isUpdating = true;

    const data = new FormData();
    data.set('update-file', this.selectedFile, this.selectedFile.name);

    try
    {
      const response = await axios.post(
        'upload/firmware',
        data,
        {
          onUploadProgress: (progressEvent: ProgressEvent) =>
          {
            this.uploadProgress = Math.round(1000 * progressEvent.loaded / progressEvent.total);
          },
        });
      console.log(response);
    }
    catch (e)
    {
      if (this.isAxiosError(e))
      {
        console.log(e.message);
      }
      else
      {
        console.log('Something went wrong: ' + e);
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

#update-file-input
{
    opacity: 0; // invisible but it's there!
    
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

