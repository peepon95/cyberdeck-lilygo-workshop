const CONFIG = {
  repo: 'https://github.com/peepon95/cyberdeck-lilygo-workshop',
  zip: 'https://github.com/peepon95/cyberdeck-lilygo-workshop/archive/refs/heads/main.zip',
  codex: 'https://chatgpt.com/download/',
  claude: 'https://claude.ai/download',
  lovable: 'https://lovable.dev/',
  platformio: 'https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html'
};

const saved = JSON.parse(localStorage.getItem('cyberDeckGuide') || '{}');
const state = { os: saved.os || '', ai: saved.ai === 'claude' ? 'claude' : 'codex', step: Number.isInteger(saved.step) ? Math.min(saved.step, 5) : 0 };
const save = () => localStorage.setItem('cyberDeckGuide', JSON.stringify(state));
const isMac = () => state.os === 'mac';
const osName = () => isMac() ? 'Mac' : 'Windows';
const shellName = () => isMac() ? 'Terminal' : 'PowerShell';

const command = (label, value) => `<div class="copy-card"><span>${label}</span><code>${value}</code><button data-copy="${encodeURIComponent(value)}">Copy</button></div>`;
const prompt = (value, target = state.ai === 'claude' ? 'CLAUDE CODE' : state.ai === 'lovable' ? 'CODEX OR CLAUDE CODE' : 'CODEX') => `<div class="prompt-card"><span>PASTE INTO ${target}</span><p>${value}</p><button data-copy="${encodeURIComponent(value)}">Copy prompt</button></div>`;
const list = (...items) => `<ol>${items.map(item => `<li>${item}</li>`).join('')}</ol>`;

const steps = [
  {
    label: 'Connect internet',
    render: () => `<p class="step-number">STEP 1 · ${osName().toUpperCase()}</p><h2>Connect your laptop to your phone hotspot.</h2>${list('Turn on Personal Hotspot on iPhone, or Wi-Fi Hotspot on Android.', `On your ${osName()}, open Wi-Fi settings.`, 'Choose your phone hotspot and enter its password.', 'Open a webpage to check that internet works.', 'Keep your phone connected to power if possible.')}<div class="note"><b>Why?</b> Your coding assistant and first software downloads need internet. Later, the Cyber Deck dashboard works locally without internet.</div>`
  },
  {
    label: 'Choose AI tool',
    render: () => `<p class="step-number">STEP 2</p><h2>Choose how you want to build.</h2><p>Choose Codex or Claude Code to set up, build and flash your LILYGO.</p><div class="tool-choice" id="aiChoice"><button data-value="codex"><b>Codex</b><span>Firmware, build and flash</span></button><button data-value="claude"><b>Claude Code</b><span>Firmware, build and flash</span></button></div><div id="toolSetup">${toolSetup()}</div>`
  },
  {
    label: 'Get starter code',
    render: () => `<p class="step-number">STEP 3</p><h2>Get the Cyber Deck starter.</h2><h3>Option A · Let your coding agent get it</h3>${list('Create an empty folder named Cyber Deck Workshop on your laptop.', 'Open that folder in Codex or Claude Code.', 'Paste the prompt below and follow its instructions.')}${prompt('Help me get the Cyber Deck starter from https://github.com/peepon95/cyberdeck-lilygo-workshop onto my laptop. Clone it into a new folder, help me open that folder as my project, and check that platformio.ini and src/main.cpp are present. If Git is missing, help me install it. Do not overwrite any existing folder or flash the board yet.')}<h3>Option B · Download a ZIP</h3><p>Prefer a download? Extract the ZIP and open the resulting project folder.</p><div class="download-row"><a class="button secondary" href="${CONFIG.zip}">Download Starter ZIP</a><a class="button quiet" href="${CONFIG.repo}" target="_blank" rel="noreferrer">View on GitHub</a></div>${list(isMac() ? 'Open Downloads in Finder and double-click the ZIP.' : 'Open Downloads in File Explorer, right-click the ZIP and choose Extract All.', 'Open the extracted cyberdeck-lilygo-workshop-main folder.', 'Check that platformio.ini and the src folder are inside.')}<p>Choose either option, then continue with the project folder you created.</p>`
  },
  {
    label: 'Open project',
    render: () => openProjectStep()
  },
  {
    label: 'Follow your agent',
    render: () => `<p class="step-number">STEP 5</p><h2>Follow your coding agent.</h2><p>Continue in Codex or Claude Code using the setup prompt from the previous step. It will guide you through installing tools, detecting the LILYGO, building and uploading.</p>${list('Keep your LILYGO connected with a USB-C data cable.', 'Follow the instructions in your coding platform, one step at a time.', 'Approve the upload when your agent asks.', 'Watch for HELLO WORLD! and MY FIRST LILYGO, followed by the Wi-Fi Setup screen.', 'Return here and choose Next step when the setup Wi-Fi details appear.')}<details><summary>If something fails</summary><p>Tell your coding agent what happened and share its full error output. Ask it to fix the issue and guide you through the next physical action.</p></details>`
  },
  {
    label: 'Open dashboard',
    render: () => `<p class="step-number">STEP 6</p><h2>Connect your Cyber Deck to Wi-Fi.</h2><img class="setup-photo" src="assets/wifi-setup-reference.png" alt="LILYGO screen showing its Wi-Fi Setup network and local address">${list('Read the unique LILYGO-SETUP name on the screen.', 'On your phone or laptop, open Wi-Fi settings.', 'Join that LILYGO-SETUP network using the password displayed on your LILYGO.', 'If it says No Internet, stay connected.', 'The My First LILYGO setup page may open automatically. If it does not, open http://192.168.4.1 in a browser.', 'Enter your phone hotspot name and password, then tap Save and connect.', 'Watch the LILYGO change to Wi-Fi Connected and show its assigned IP address.')}<img class="setup-photo" src="assets/wifi-portal-reference.png" alt="My First LILYGO local setup page with hotspot name, password and Save and connect button"><p>The page above is hosted by your LILYGO. Enter your phone hotspot details here. “Trying to connect” means it is still attempting to join; confirm success on the physical display.</p><div class="credentials"><span>WI-FI</span><b>LILYGO-SETUP-XXXXXX</b><span>PASSWORD</span><b>Use the password shown on your screen</b><span>SETUP PAGE</span><b>192.168.4.1</b></div><details><summary>Want an easier setup password?</summary><p>Ask your coding agent to change the setup Wi-Fi password and its LCD label together. Use at least 8 characters.</p>${prompt("Help me change the LILYGO setup access-point password to an easier password I choose, with at least 8 characters. Update the password shown on the LCD to match, preserve hotspot credentials and display pins, build with PlatformIO, and ask before flashing.")}</details><div class="note"><b>Next time:</b> the saved network reconnects automatically. If it cannot connect, the LILYGO setup network remains available so you can try again.</div>`
  }
];

function toolSetup() {
  if (state.ai === 'lovable') {
    return `<div class="tool-panel"><span class="partner-chip">SUPPORTING WORKSHOP PARTNER</span><h3>Design with Lovable</h3><p>Use Lovable to create the phone dashboard or visual concept. You will still use Codex or Claude Code for ESP32 firmware, PlatformIO and USB flashing.</p><a class="button secondary" href="${CONFIG.lovable}" target="_blank" rel="noreferrer">Open Lovable ↗</a></div>`;
  }
  return state.ai === 'claude'
    ? `<div class="tool-panel"><h3>Install Claude Code</h3><p>Download Claude, sign in, then verify the command-line tool.</p><a class="button secondary" href="${CONFIG.claude}" target="_blank" rel="noreferrer">Official Claude download ↗</a>${isMac() ? command('Terminal', 'curl -fsSL https://claude.ai/install.sh | bash') : command('PowerShell', 'irm https://claude.ai/install.ps1 | iex')}${command(shellName(), 'claude --version')}</div>`
    : `<div class="tool-panel"><h3>Install Codex</h3><p>Download the Codex desktop app, install it and sign in.</p><a class="button secondary" href="${CONFIG.codex}" target="_blank" rel="noreferrer">Official Codex download ↗</a></div>`;
}

function openProjectStep() {
  if (state.ai === 'lovable') {
    const lovablePrompt = 'Design a mobile-first local dashboard for my LILYGO Cyber Deck. Use a playful retro pixel-art style, large touch-friendly controls and a responsive phone layout. It must be a visual prototype that can later be simplified into HTML, CSS and JavaScript hosted by an ESP32. Do not use a cloud database, login, external API or runtime CDN. Do not attempt to compile or flash hardware.';
    return `<p class="step-number">STEP 4</p><h2>Prototype the interface in Lovable.</h2>${list('Open Lovable and create a new project.', 'Copy the prompt below and generate the first interface.', 'Keep the design focused on one small pet, game or dashboard.', 'When the design is ready, open the downloaded Cyber Deck starter in Codex or Claude Code.', 'Ask that coding agent to recreate only the approved lightweight interface inside the ESP32 project.')} ${prompt(lovablePrompt, 'LOVABLE')}<div class="note"><b>Important:</b> Lovable designs the interface; Codex or Claude Code integrates it with the existing firmware and PlatformIO. Do not let two tools edit the same files at the same time.</div>`;
  }
  if (state.ai === 'claude') {
    return `<p class="step-number">STEP 4</p><h2>Open the folder in Claude Code.</h2>${list(`Open ${shellName()}.`, 'Move into the extracted project folder.', 'Start Claude Code from that folder.', 'Plug in your LILYGO using a USB-C data cable.', 'Paste the setup prompt below and follow the agent step by step.')}${command(shellName(), isMac() ? 'cd ~/Downloads/cyberdeck-lilygo-workshop-main && claude' : 'cd $HOME\\Downloads\\cyberdeck-lilygo-workshop-main; claude')}${prompt("Help me set up this Cyber Deck starter on my connected LILYGO T-Display-S3. Inspect the project, check whether PlatformIO is installed, and identify the correct USB serial port. Install any missing build tools, build the firmware and fix any errors. Preserve the existing display configuration and GPIO pins. Ask me before flashing. After uploading, guide me through checking the Hello World screen and connecting to the Wi-Fi setup page. Explain only what I need to do, one step at a time.")}`;
  }
  return `<p class="step-number">STEP 4</p><h2>Open the folder in Codex.</h2>${list('Open the Codex app.', 'Choose Open Folder.', 'Select cyberdeck-lilygo-workshop-main from Downloads.', 'Start a new chat for this project.', 'Plug in your LILYGO using a USB-C data cable.', 'Paste the setup prompt below and follow the agent step by step.')}${prompt("Help me set up this Cyber Deck starter on my connected LILYGO T-Display-S3. Inspect the project, check whether PlatformIO is installed, and identify the correct USB serial port. Install any missing build tools, build the firmware and fix any errors. Preserve the existing display configuration and GPIO pins. Ask me before flashing. After uploading, guide me through checking the Hello World screen and connecting to the Wi-Fi setup page. Explain only what I need to do, one step at a time.")}`;
}

const projects = [
  ['PG', 'PetGotchi', 'Create your own pet companion with feeding, sleep, happiness and different expressions.'],
  ['RR', 'Rocket Rush', 'Move a neon spaceship with two buttons while it automatically shoots aliens.'],
  ['SN', 'Pixel Snake', 'Guide a growing pixel snake, collect snacks and avoid hitting its tail.'],
  ['CS', 'Catch the Stars', 'Move a basket left and right to catch stars before they fall.'],
  ['PR', 'Pixel Racer', 'Steer a tiny car between obstacles and chase a high score.'],
  ['SB', 'Study Buddy', 'Add a Pomodoro timer, break reminders and encouraging messages.'],
  ['NB', 'Digital Name Badge', 'Show custom names, colours, pixel art and scrolling animation.'],
  ['??', 'Your Own Idea', 'Invent an original pet, useful tool, artwork or two-button game.']
];

const helps = [
  ['No serial port', 'Try another USB-C data cable and USB port.'],
  ['Build failed', 'Copy the full error into your coding assistant and ask for the smallest safe fix.'],
  ['Upload failed', 'Check the port, then try BOOT + RST recovery mode.'],
  ['Blank screen', 'Press RST once. If still blank, restore the starter firmware.'],
  ['LILYGO setup Wi-Fi missing', 'Keep the board powered, press RST and wait ten seconds.'],
  ['192.168.4.1 will not open', 'Stay on LILYGO-SETUP Wi-Fi and type http://192.168.4.1 exactly.']
];

function setChoices() {
  document.querySelectorAll('#osChoice button').forEach(button => button.setAttribute('aria-pressed', button.dataset.value === state.os));
  document.querySelectorAll('#aiChoice button').forEach(button => button.setAttribute('aria-pressed', button.dataset.value === state.ai));
}

function renderStep() {
  document.querySelector('#workshop').hidden = !state.os;
  if (!state.os) return;
  document.querySelector('#stepContent').innerHTML = steps[state.step].render();
  document.querySelector('#stepDots').innerHTML = steps.map((step, index) => `<button class="${index === state.step ? 'active' : ''}" data-step="${index}"><span>${index + 1}</span>${step.label}</button>`).join('');
  document.querySelector('#previousStep').disabled = state.step === 0;
  document.querySelector('#nextStep').textContent = state.step === steps.length - 1 ? 'See project ideas ↓' : 'Next step →';
  document.querySelectorAll('#stepDots button').forEach(button => button.onclick = () => goTo(+button.dataset.step));
  document.querySelectorAll('[data-copy]').forEach(button => button.onclick = () => copy(decodeURIComponent(button.dataset.copy)));
  document.querySelectorAll('#aiChoice button').forEach(button => button.onclick = () => { state.ai = button.dataset.value; save(); renderStep(); });
  setChoices();
}

function goTo(index) {
  state.step = Math.max(0, Math.min(steps.length - 1, index)); save(); renderStep();
  document.querySelector('#workshop').scrollIntoView({ behavior: 'smooth', block: 'start' });
  document.querySelector('#stepContent').focus({ preventScroll: true });
}

async function copy(value) {
  try { await navigator.clipboard.writeText(value); }
  catch { const area = document.createElement('textarea'); area.value = value; document.body.append(area); area.select(); document.execCommand('copy'); area.remove(); }
  const toast = document.querySelector('#toast'); toast.textContent = 'Copied!'; toast.classList.add('show'); setTimeout(() => toast.classList.remove('show'), 1400);
}

document.querySelectorAll('#osChoice button').forEach(button => button.onclick = () => { state.os = button.dataset.value; state.step = 0; save(); setChoices(); renderStep(); document.querySelector('#workshop').scrollIntoView({ behavior: 'smooth' }); });
document.querySelector('#previousStep').onclick = () => goTo(state.step - 1);
document.querySelector('#nextStep').onclick = () => state.step === steps.length - 1 ? document.querySelector('#ideas').scrollIntoView({ behavior: 'smooth' }) : goTo(state.step + 1);
document.querySelector('#projectGrid').innerHTML = projects.map((project, index) => `<article><span>${project[0]}</span><h3>${project[1]}</h3><p>${project[2]}</p><button data-project="${index}">Copy project prompt</button></article>`).join('');
document.querySelectorAll('[data-project]').forEach(button => button.onclick = () => {
  const project = projects[+button.dataset.project];
  const text = state.ai === 'lovable'
    ? `Create a mobile-first visual prototype for ${project[1]}: ${project[2]} Use a retro pixel-art style and large touch controls. Keep it suitable for a 320x170 LILYGO Cyber Deck companion interface. Use no login, cloud database, external API or runtime CDN. This is an interface prototype only; do not attempt to compile or flash hardware.`
    : `Inspect my existing Cyber Deck LILYGO T-Display-S3 PlatformIO project first. Help me build ${project[1]}: ${project[2]} Preserve the verified 320x170 display configuration, Wi-Fi, two button GPIOs and existing working features. Build with PlatformIO, explain how to test it, and ask before flashing or publishing.`;
  copy(text);
});
document.querySelector('#helpGrid').innerHTML = helps.map(item => `<details><summary>${item[0]}</summary><p>${item[1]}</p></details>`).join('');
setChoices(); renderStep();

if ('serviceWorker' in navigator && location.protocol.startsWith('http')) navigator.serviceWorker.register('./sw.js').catch(() => {});
