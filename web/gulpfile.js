const gulp = require('gulp'),
    debug = require('gulp-debug'),
    clean = require('gulp-clean'),
    sass = require('gulp-sass'),
    postcss = require('gulp-postcss'),
    header = require('gulp-header'),
    cleanCSS = require('gulp-clean-css'),
    rtlcss = require('gulp-rtlcss'),
    minifyJS = require('gulp-minify'),
    rename = require('gulp-rename'),
    purgecss = require('gulp-purgecss'),
    rollupStream = require('@rollup/stream'),
    rollupBabel = require('rollup-plugin-babel'),
    rollupCleanup = require('rollup-plugin-cleanup'),
    { nodeResolve } = require('@rollup/plugin-node-resolve'),
    rollupCommonjs = require('@rollup/plugin-commonjs'),
    rollupReplace = require('@rollup/plugin-replace'),
    vinylSource = require('vinyl-source-stream'),
    vinylBuffer = require('vinyl-buffer'),
    critical = require('critical').stream,
    browserSync = require('browser-sync'),
    glob = require('glob'),
    spawn = require('cross-spawn'),
    fs = require('fs'),
    path = require('path'),
    YAML = require('yaml'),
    yargs = require('yargs/yargs'),
    cp = require('child_process'),
    pkg = require('./package.json'),
    year = new Date().getFullYear(),
    argv = yargs(process.argv).argv;


let BUILD = false,
    distDir = './.tmp',
    webAppDir = './.tmp',
    srcDir = './src';

/**
 * Enable BUILD mode and set directories
 */
gulp.task('build-on', (cb) => {
    BUILD = true;
    distDir = './data';
    webAppDir = './data';

    cb();
});

/**
 * Return banner added to CSS and JS dist files
 */
const getBanner = () => {
    return `/*!
* Aegir v${pkg.version} (${pkg.homepage})
* @version ${pkg.version}
* @link ${pkg.homepage}
* Copyright 2021-${year} Sorin Otescu
* Licensed under MIT (https://github.com/sorinescu/Aegir/blob/main/LICENSE)
*/
`;
};

/**
 * Array.flat polyfill
 */
if (!Array.prototype.flat) {
    Object.defineProperty(Array.prototype, 'flat', {
        value: function(depth = 1) {
            return this.reduce(function(flat, toFlatten) {
                return flat.concat((Array.isArray(toFlatten) && (depth > 1)) ? toFlatten.flat(depth - 1) : toFlatten);
            }, []);
        }
    });
}

/**
 * Check unused Jekyll partials
 */
gulp.task('unused-files', (cb) => {
    let foundFiles = [];

    glob.sync(`${srcDir}/pages/**/*.{html,md}`).forEach((file) => {
        let fileContent = fs.readFileSync(file);

        fileContent.toString().replace(/\{% include(_cached)? ([a-z0-9\/_-]+\.html)/g, (f, c, filename) => {
            filename = `${srcDir}/pages/_includes/${filename}`;

            if (!foundFiles.includes(filename)) {
                foundFiles.push(filename);
            }
        });
    });

    let includeFiles = glob.sync(`${srcDir}/pages/_includes/**/*.html`);

    includeFiles.forEach((file) => {
        if (!foundFiles.includes(file)) {
            console.log('file', file);
        }
    });

    cb();
});

/**
 * Clean `dist` folder before build
 */
gulp.task('clean-dirs', () => {
    return gulp
        .src(`{${distDir}/*,${webAppDir}/*}`, { read: false })
        .pipe(clean());
});

gulp.task('clean-jekyll', (cb) => {
    return spawn('bundle', ['exec', 'jekyll', 'clean'], { stdio: 'inherit' })
        .on('close', cb);
});

/**
 * Compile SASS to CSS and move it to dist directory
 */
gulp.task('sass', () => {
    return gulp
        .src(`${srcDir}/scss/!(_)*.scss`)
        .pipe(debug())
        .pipe(sass({
            style: 'expanded',
            precision: 7,
            importer: (url, prev, done) => {
                if (url[0] === '~') {
                    url = path.resolve('node_modules', url.substr(1));
                }

                return { file: url };
            },
        }).on('error', sass.logError))
        .pipe(postcss([
            require('autoprefixer'),
        ]))
        .pipe(gulp.dest(`${distDir}/css/`))
        .pipe(browserSync.reload({
            stream: true,
        }))
        .pipe(rtlcss())
        .pipe(rename((path) => {
            path.basename += '.rtl';
        }))
        .pipe(gulp.dest(`${distDir}/css/`));
});

/**
 * CSS minify
 */
gulp.task('css-minify', function() {
    return gulp.src(`${distDir}/css/!(*.min).css`)
        .pipe(debug())
        .pipe(cleanCSS())
        .pipe(rename((path) => {
            path.basename += '.min';
        }))
        .pipe(gulp.dest(`${distDir}/css/`));
});


/**
 * Compile JS files to dist directory
 */
let cache;
gulp.task('js', () => {
    const g = rollupStream({
            input: `${srcDir}/js/aegir.js`,
            cache,
            output: {
                name: 'aegir.js',
                format: 'umd',
            },
            plugins: [
                rollupReplace({
                    'process.env.NODE_ENV': JSON.stringify(BUILD ? 'production' : 'development'),
                }),
                rollupBabel({
                    exclude: 'node_modules/**'
                }),
                nodeResolve(),
                rollupCommonjs(),
                rollupCleanup()
            ]
        })
        .on('bundle', (bundle) => {
            cache = bundle;
        })
        .pipe(vinylSource('aegir.js'))
        .pipe(vinylBuffer())
        .pipe(rename((path) => {
            path.dirname = '';
        }))
        .pipe(gulp.dest(`${distDir}/js/`))
        .pipe(browserSync.reload({
            stream: true,
        }));

    if (BUILD) {
        g.pipe(minifyJS({
                ext: {
                    src: '.js',
                    min: '.min.js'
                },
            }))
            .pipe(gulp.dest(`${distDir}/js/`));
    }

    return g;
});

/**
 * Watch Jekyll files and build it to demo directory
 */
gulp.task('watch-jekyll', (cb) => {
    browserSync.notify('Building Jekyll');
    return spawn('bundle', ['exec', 'jekyll', 'build', '--watch', '--destination', webAppDir, '--trace'], { stdio: 'inherit' })
        .on('close', cb);
});

/**
 * Build Jekyll files do demo directory
 */
gulp.task('build-jekyll', (cb) => {
    var env = Object.create(process.env);

    if (argv.preview) {
        env.JEKYLL_ENV = 'preview';
    } else {
        env.JEKYLL_ENV = 'production';
    }

    return spawn('bundle', ['exec', 'jekyll', 'build', '--destination', webAppDir, '--trace'], { env: env, stdio: 'inherit' })
        .on('close', cb);
});

// gulp.task('build-cleanup', () => {
//     return gulp
//         .src(`${webAppDir}/redirects.json`, { read: false, allowEmpty: true })
//         .pipe(clean());
// });

// gulp.task('build-purgecss', (cb) => {
//     if (argv.preview) {
//         return gulp.src('demo/dist/{libs,css}/**/*.css')
//             .pipe(purgecss({
//                 content: ['demo/**/*.html']
//             }))
//             .pipe(gulp.dest('demo/dist/css'))
//     }

//     cb();
// });

/**
 * Watch JS and SCSS files
 */
gulp.task('watch', (cb) => {
    gulp.watch('./src/scss/**/*.scss', gulp.series('sass'));
    gulp.watch('./src/js/**/*.js', gulp.series('js'));
    cb();
});

/**
 * Create BrowserSync server
 */
gulp.task('browser-sync', () => {
    browserSync({
        watch: true,
        server: {
            baseDir: webAppDir,
            routes: {
                '/node_modules': 'node_modules',
                '/css': `${distDir}/css`,
                '/js': `${distDir}/js`,
                '/img': `${srcDir}/img`,
                '/static': `${srcDir}/static`,
            },
        },
        port: 3000,
        open: false,
        host: 'localhost',
        notify: false,
        reloadOnRestart: true
    });
});

/**
 * Copy libs used in tabler from npm to dist directory
 */
gulp.task('copy-libs', (cb) => {
    const allLibs = require(`${srcDir}/pages/_data/libs`);

    let files = [];

    Object.keys(allLibs.js).forEach((lib) => {
        files.push(Array.isArray(allLibs.js[lib]) ? allLibs.js[lib] : [allLibs.js[lib]]);
    });

    Object.keys(allLibs.css).forEach((lib) => {
        files.push(Array.isArray(allLibs.css[lib]) ? allLibs.css[lib] : [allLibs.css[lib]]);
    });

    files = files.flat();

    files.forEach((file) => {
        if (!file.match(/^https?/)) {
            let dirname = path.dirname(file).replace('@', '');
            let cmd = `mkdir -p "${distDir}/dist/libs/${dirname}" && cp -r node_modules/${file} ${distDir}/dist/libs/${file.replace('@', '')}`;
            console.log(cmd);
            cp.exec(cmd)
        }
    });

    cb();
});

/**
 * Copy static files to build directory
 */
gulp.task('copy-static', () => {
    return gulp
        .src(`${srcDir}/static/**/*`)
        .pipe(gulp.dest(`${webAppDir}/static`));
});

/**
 * Add banner to build JS and CSS files
 */
gulp.task('add-banner', () => {
    return gulp.src(`${distDir}/{css,js}/**/*.{js,css}`)
        .pipe(header(getBanner()))
        .pipe(gulp.dest(`${distDir}`))
});

gulp.task('clean', gulp.series('clean-dirs', 'clean-jekyll'));

gulp.task('start', gulp.series('clean', 'sass', 'js', 'copy-libs', 'build-jekyll', gulp.parallel('watch-jekyll', 'watch', 'browser-sync')));

gulp.task('build', gulp.series('build-on', 'build-jekyll', 'clean', 'sass', 'css-minify', 'js', 'copy-static', 'copy-libs', 'add-banner', /* 'build-cleanup', 'build-purgecss'*/ ));
