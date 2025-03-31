<script>
  import Router from 'svelte-spa-router';
  import routes from './routes';
  import Sidebar from './components/Sidebar.svelte';
  import Header from './components/Header.svelte';
</script>

<main>
  <Header />
  <div class="container">
    <Sidebar />
    <div class="content">
      <Router {routes} />
    </div>
  </div>
</main>

<style>
  :global(body) {
    margin: 0;
    padding: 0;
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen,
      Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
    background-color: #f5f5f5;
  }

  .container {
    display: flex;
    height: calc(100vh - 60px);
  }

  .content {
    flex: 1;
    padding: 20px;
    overflow-y: auto;
  }
</style> 