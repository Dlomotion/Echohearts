# Echohearts
My web page For Eco_kin
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Echohearts: Veridian Keepers - Official Site</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: 'Roboto', sans-serif;
            background-color: #0c0a09; /* stone-950 */
            color: #e7e5e4; /* stone-200 */
        }
        .font-orbitron {
            font-family: 'Orbitron', sans-serif;
        }
        .hero-section {
            background-image: linear-gradient(to bottom, rgba(10, 10, 10, 0.4), rgba(10, 10, 10, 1)), url('image_a95a5a.jpg');
            background-size: cover;
            background-position: center;
        }
        .glass-panel {
            background: rgba(17, 24, 39, 0.7);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(56, 189, 248, 0.2);
        }
        .feature-card, .rival-card {
            transition: all 0.3s ease;
            border: 1px solid rgba(56, 189, 248, 0.1);
        }
        .feature-card:hover, .rival-card:hover {
            transform: translateY(-8px);
            border-color: rgba(56, 189, 248, 0.4);
            box-shadow: 0 8px 25px rgba(56, 189, 248, 0.1);
        }
        .btn-primary {
            background: linear-gradient(45deg, #14b8a6, #38bdf8); /* teal-500 to sky-400 */
            transition: all 0.3s ease;
        }
        .btn-primary:hover {
            transform: scale(1.05);
            box-shadow: 0 0 20px rgba(56, 189, 248, 0.5);
        }
        .section-divider {
            height: 2px;
            background: linear-gradient(90deg, transparent, rgba(56, 189, 248, 0.4), transparent);
        }
        .gallery-img {
            transition: transform 0.3s ease;
        }
        .gallery-img:hover {
            transform: scale(1.05);
        }
    </style>
</head>
<body class="w-full">

    <!-- Hero Section -->
    <section class="hero-section min-h-screen flex flex-col items-center justify-center text-center p-8">
        <div class="max-w-4xl">
            <img src="image_a9700b.jpg" alt="Echohearts Logo" class="w-full max-w-md mx-auto mb-4 rounded-lg shadow-2xl">
            <h1 class="font-orbitron text-5xl md:text-7xl font-black text-white uppercase" style="text-shadow: 0 0 20px rgba(0,0,0,0.7);">Echohearts</h1>
            <h2 class="font-orbitron text-2xl md:text-3xl font-bold text-emerald-300 uppercase tracking-widest" style="text-shadow: 0 0 15px rgba(0,0,0,0.7);">Veridian Keepers</h2>
            <p class="mt-6 text-xl md:text-2xl text-slate-100 max-w-3xl mx-auto" style="text-shadow: 0 2px 5px rgba(0,0,0,0.7);">
                In a world scarred by shadow, your bond is their only hope. Capture, nurture, and evolve over 550 unique Echo-Kin to restore the soul of Aethelgard.
            </p>
            <div class="mt-12">
                <a href="#signup" class="btn-primary text-white font-bold text-xl py-4 px-10 rounded-full uppercase tracking-wider">Join the Keepers</a>
            </div>
        </div>
    </section>

    <!-- Main Content -->
    <div class="bg-stone-950 py-20">
        <div class="max-w-7xl mx-auto px-6 lg:px-8 space-y-24">

            <!-- The Story So Far -->
            <section class="text-center">
                <h2 class="font-orbitron text-4xl font-bold text-white">A World's Soul Hangs in the Balance</h2>
                <div class="section-divider max-w-sm mx-auto my-6"></div>
                <p class="max-w-3xl mx-auto text-lg text-slate-300 leading-relaxed">
                    The vibrant world of Aethelgard is dying. A cosmic blight known as **The Gloom** seeps into the land, corrupting the noble Echo-Kin and twisting them into monstrous Aberrants. Under the iron fist of the tyrannical **Echolord Vorlag**, hope fades. But a new generation of Veridian Keepers is rising—those with the rare gift of **Echo Resonance**, who can form profound bonds with Kin and push back the darkness.
                </p>
            </section>
            
            <!-- Meet the Echo-Kin Gallery -->
            <section>
                <h2 class="font-orbitron text-4xl font-bold text-white text-center">Discover the Echo-Kin</h2>
                <div class="section-divider max-w-sm mx-auto my-6"></div>
                <div class="grid grid-cols-2 md:grid-cols-4 gap-4 mt-12">
                    <img src="image_a96c63.jpg" alt="Piplin" class="gallery-img rounded-lg shadow-lg aspect-w-1 aspect-h-1 w-full h-full object-cover">
                    <img src="image_a96920.jpg" alt="Elemental Starters" class="gallery-img rounded-lg shadow-lg aspect-w-1 aspect-h-1 w-full h-full object-cover">
                    <img src="image_a96d43.jpg" alt="Chil-Kin Group" class="gallery-img rounded-lg shadow-lg aspect-w-1 aspect-h-1 w-full h-full object-cover">
                    <img src="image_a96944.jpg" alt="Mythic Kin Battle" class="gallery-img rounded-lg shadow-lg aspect-w-1 aspect-h-1 w-full h-full object-cover">
                </div>
            </section>

            <!-- Core Features Section -->
            <section>
                <div class="grid grid-cols-1 md:grid-cols-3 gap-8 text-center">
                    <div class="feature-card glass-panel p-8 rounded-xl">
                        <h3 class="font-orbitron text-2xl font-bold text-white">DEEP BONDING</h3>
                        <p class="mt-2 text-slate-400">Nurture your Chil-Kin in lush gardens. Your care directly impacts their evolution, personality, and power.</p>
                    </div>
                    <div class="feature-card glass-panel p-8 rounded-xl">
                        <h3 class="font-orbitron text-2xl font-bold text-white">STRATEGIC COMBAT</h3>
                        <p class="mt-2 text-slate-400">Master an 18-type elemental system in tactical turn-based duels, co-op raids, and massive guild wars.</p>
                    </div>
                    <div class="feature-card glass-panel p-8 rounded-xl">
                        <h3 class="font-orbitron text-2xl font-bold text-white">ENDLESS EXPLORATION</h3>
                        <p class="mt-2 text-slate-400">Discover new frontiers, from the corrupted Umbral Mire to the cosmic Crimson Scars of Mars.</p>
                    </div>
                </div>
            </section>

            <!-- Rivals Section -->
            <section>
                 <h2 class="font-orbitron text-4xl font-bold text-white text-center">Meet Your Rivals</h2>
                <div class="section-divider max-w-sm mx-auto my-6"></div>
                 <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-5 gap-6 mt-12">
                    <div class="rival-card glass-panel p-6 rounded-lg text-center">
                        <h3 class="text-xl font-bold text-yellow-300">Lysander</h3>
                        <p class="text-sm text-yellow-400/70">The Prodigy</p>
                        <p class="text-xs mt-2">"Show me you're worthy of my time."</p>
                    </div>
                     <div class="rival-card glass-panel p-6 rounded-lg text-center">
                        <h3 class="text-xl font-bold text-blue-300">Seraphina</h3>
                        <p class="text-sm text-blue-400/70">The Historian</p>
                        <p class="text-xs mt-2">"The old ways must be respected."</p>
                    </div>
                     <div class="rival-card glass-panel p-6 rounded-lg text-center">
                        <h3 class="text-xl font-bold text-orange-400">Kael</h3>
                        <p class="text-sm text-orange-500/70">The Scavenger</p>
                        <p class="text-xs mt-2">"Talk is cheap. Let your Kin do the talking."</p>
                    </div>
                     <div class="rival-card glass-panel p-6 rounded-lg text-center">
                        <h3 class="text-xl font-bold text-purple-400">Zira</h3>
                        <p class="text-sm text-purple-500/70">The Enigma</p>
                        <p class="text-xs mt-2">"Your path is but one thread in the weave."</p>
                    </div>
                     <div class="rival-card glass-panel p-6 rounded-lg text-center">
                        <h3 class="text-xl font-bold text-green-400">Jax</h3>
                        <p class="text-sm text-green-500/70">The Opportunist</p>
                        <p class="text-xs mt-2">"There's always an angle to be played."</p>
                    </div>
                 </div>
            </section>

            <!-- Signup Section -->
            <section id="signup" class="glass-panel rounded-2xl p-8 md:p-12">
                <div class="text-center">
                    <h2 class="font-orbitron text-4xl font-bold text-white">Your Echo Awaits</h2>
                    <p class="mt-4 max-w-2xl mx-auto text-lg text-slate-300">
                        Sign up to become a Veridian Keeper. Be the first to receive exclusive news, development updates, and an invitation to the beta.
                    </p>
                    <form id="signup-form" class="mt-8 max-w-lg mx-auto flex flex-col sm:flex-row gap-4">
                        <input type="email" id="email-input" required placeholder="Enter your email address" class="w-full flex-grow bg-slate-800 border border-slate-600 rounded-full px-6 py-3 text-white focus:ring-2 focus:ring-sky-400 focus:outline-none">
                        <button type="submit" class="btn-primary text-white font-bold py-3 px-8 rounded-full uppercase tracking-wider">Enlist Now</button>
                    </form>
                    <p id="success-message" class="mt-4 text-green-400 hidden">Thank you, Keeper! Your Resonance has been registered.</p>
                </div>
            </section>

        </div>
    </div>

    <!-- Footer -->
    <footer class="bg-gray-900/50 py-8 text-center border-t border-gray-800">
        <p class="text-slate-400">An <span class="font-bold text-white">Into Deep Studios</span> Production</p>
        <p class="text-xs text-slate-500 mt-1">Based in South Holland, Illinois</p>
        <p class="text-xs text-slate-500 mt-2">Lead Designer: Donta Owens</p>
        <div class="flex justify-center gap-6 mt-4">
            <a href="#" class="text-slate-400 hover:text-white transition">Discord</a>
            <a href="#" class="text-slate-400 hover:text-white transition">Twitter/X</a>
            <a href="#" class="text-slate-400 hover:text-white transition">YouTube</a>
        </div>
    </footer>

    <script>
        document.getElementById('signup-form').addEventListener('submit', function(event) {
            event.preventDefault();
            const emailInput = document.getElementById('email-input');
            const successMessage = document.getElementById('success-message');
            
            if (emailInput.value) {
                console.log('Email submitted:', emailInput.value);
                emailInput.value = '';
                successMessage.classList.remove('hidden');
                setTimeout(() => {
                    successMessage.classList.add('hidden');
                }, 5000);
            }
        });

        document.querySelector('a[href="#signup"]').addEventListener('click', function(e) {
            e.preventDefault();
            document.querySelector('#signup').scrollIntoView({
                behavior: 'smooth'
            });
        });
    </script>

</body>
</html>
