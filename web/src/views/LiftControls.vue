<template>
  <div class="card fluid home">
    <h2 class="section double-padded">Lift Controls</h2>
    <div class="section double-padded">
      <div class="container">

        base: {{ baseUri }}
        status: {{ status }}
        online: {{ isLiftOnline }}

        <template v-if="isLiftOnline">
          <div class="row">
            <div class="col-sm">
              Speed: {{ currentSpeed }}
            </div>
          </div>

          <div class="row">
            <div class="col-sm-4 col-sm-offset-4">
              <button v-on:click="liftGoUp"><arrow-up-icon size="5x"/></button>
            </div>
          </div>

          <div class="row">
            <div class="col-sm-4">
              <button v-on:click="liftFaster"><minus-icon size="5x"/></button>
            </div>
            <div class="col-sm-4">
              <button v-on:click="liftStop"><square-icon size="5x" class="stop-button"/></button>
            </div>
            <div class="col-sm-4">
              <button v-on:click="liftSlower"><plus-icon size="5x"/></button>
            </div>
          </div>

          <div class="row">
            <div class="col-sm-4 col-sm-offset-4">
              <button v-on:click="liftGoDown"><arrow-down-icon size="5x"/></button>
            </div>
          </div>
        </template>

        <template v-else>
          <div class="row">
            <div class="col-sm-4 col-sm-offset-4">
              <div class="card fluid error">
                <h3>Error</h3>
                <p>Lift is not online!</p>
              </div>
            </div>
          </div>
        </template>
       
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from "vue-property-decorator";
import { ArrowUpIcon, ArrowDownIcon, SquareIcon, PlusIcon, MinusIcon } from "vue-feather-icons";
import Axios, { AxiosInstance } from "axios";

interface StatusMessage
{
  status: string;
}

interface SpeedMessage
{
  speed: number;
}

@Component({
  components: {
    ArrowUpIcon,
    ArrowDownIcon,
    SquareIcon,
    PlusIcon,
    MinusIcon
  }
})
export default class LiftControls extends Vue
{
  private axios: AxiosInstance;

  private baseUri: string = process.env.NODE_ENV === "development" ? "http://tvlift.local/api/" : "api/";
  private currentSpeed: number = 0;
  private status: string = "offline";

  get isLiftOnline(): boolean
  {
    return this.status === "online";
  }

  constructor()
  {
    super();

    this.axios = Axios.create({
      baseURL: this.baseUri
    });
  }

  private mounted(): void
  {
    this.updateStatus();
    this.updateSpeed();
  }

  private liftGoUp(): void
  {
    this.axios.post("lift/up", <SpeedMessage>{speed: this.currentSpeed });
  }

  private liftGoDown(): void
  {
    this.axios.post("lift/down", <SpeedMessage>{speed: this.currentSpeed });
  }

  private liftStop(): void
  {
    this.axios.post("lift/stop");
  }

  private liftSlower(): void
  {
    this.updateSpeed()
      .then(currentSpeed => this.axios.post("lift/speed", <SpeedMessage>{speed: currentSpeed / 2}))
      .catch(e =>
      {
        console.error(e);
      })
      .then(this.updateSpeed);
  }

  private liftFaster(): void
  {
    this.updateSpeed()
      .then(currentSpeed => this.axios.post("lift/speed", <SpeedMessage>{speed: currentSpeed * 2}))
      .catch(e =>
      {
        console.error(e);
      })
      .then(this.updateSpeed);
  }

  private updateStatus(): Promise<void>
  {
    return this.axios.get<StatusMessage>("lift/status")
          .then(x =>
          {
            this.status = x.data.status;
          })
          .catch(e =>
          {
            console.error(e);
          });
  }
  
  private updateSpeed(): Promise<number>
  {
    return this.axios.get<SpeedMessage>("lift/speed")
          .then(x =>
          {
            this.currentSpeed = x.data.speed;
            return this.currentSpeed;
          })
          .catch(e =>
          {
            console.error(e);
            return this.currentSpeed;
          });
  }
}
</script>

<style lang="scss">
.home
{
  text-align: center;
}

.stop-button
{
  color: var(--error-color);
}
</style>

