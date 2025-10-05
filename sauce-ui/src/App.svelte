<script lang="ts">
  let ip: string | null = $state(null);
  let connectPromise: Promise<void> | null = $state(null);
  let error: string | null = $state(null);

  async function connect(addr: string) {
    let resp;
    let data;
    
    try {
       resp = await fetch(`http://${addr}/`);
       
       if (!resp.ok) {
         connectPromise = null;
         error = `Failed to connect to ${addr}: ${resp.status} ${resp.statusText}`;
         return;
       }

       data = await resp.json();
    } catch (error) {
      connectPromise = null;
      error = `Failed to connect to ${addr}: ${error}`;
      return;
    }

    if (data['superposition'] != "v1") {
      connectPromise = null;
      error = `Failed to connect to ${addr}: Invalid version response, expected {"superposition":"v1"}, got ${data}`;
      return;
    }

    ip = addr;
    connectPromise = null;
  }

  function onConnectPressed(event: SubmitEvent) {
    event.preventDefault();

    const formData = new FormData(event.target as HTMLFormElement);
    error = null;
    connectPromise = connect(formData.get('ip') as string);
  }

  async function gamble() : Promise<{ dispensing: "left" | "right"; roll: number, secs_until: number }> {
    return await fetch(`http://${ip}/gamble`).then((resp) => {
      if (!resp.ok) {
        throw new Error(`Failed to gamble: ${resp.status} ${resp.statusText}`);
      }
      return resp.json();
    });
  }

  async function getState(): Promise<{ odds: number }> {
    return await fetch(`http://${ip}/state`).then((resp) => {
      if (!resp.ok) {
        throw new Error(`Failed to getState: ${resp.status} ${resp.statusText}`);
      }
      return resp.json();
    });
  }
</script>

<main>
  {#if ip}
    <h1>Connected to IP: {ip}</h1>
    <button onclick={() => ip = null}>Disconnect</button>
  {:else}
    {#if error != null}
      <p style="color: red">{error}</p>
    {/if}

    {#if connectPromise}
      <p>Connecting...</p>
      <button onclick={() => connectPromise = null}>Cancel</button>
    {/if}

    <!-- <input type="text" bind:value={ip} placeholder="Enter your IP address" /> -->
     <form onsubmit={onConnectPressed}>
      <div>
        <label for="name">Smart Saucinitizer Dispenser™ IP Address</label>
        <input type="text" id="name" name="ip" disabled={connectPromise != null} />
      </div>
      <button type="submit" disabled={connectPromise != null}>Connect</button>
    </form>
  {/if}
</main>