import Dashboard from './pages/Dashboard.svelte';
import Balance from './pages/Balance.svelte';
import Register from './pages/Register.svelte';
import Budget from './pages/Budget.svelte';
import Cleared from './pages/Cleared.svelte';
import AddEntry from './pages/AddEntry.svelte';
import EditEntry from './pages/EditEntry.svelte';

export default {
  '/': Dashboard,
  '/balance': Balance,
  '/register': Register,
  '/budget': Budget,
  '/cleared': Cleared,
  '/add': AddEntry,
  '/edit/:date/:payee': EditEntry
}; 