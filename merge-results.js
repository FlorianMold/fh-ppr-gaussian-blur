const fs = require('fs');
const path = require('path');
const os = require("os");

/**
 * Promise all
 */
function promiseAllP(items, block) {
    const promises = [];
    items.forEach(function (item, index) {
        promises.push(function (item, i) {
            return new Promise(function (resolve, reject) {
                return block.apply(this, [item, index, resolve, reject]);
            });
        }(item, index))
    });
    return Promise.all(promises);
} //promiseAll

/**
 * read files
 * @param dirname string
 * @return Promise
 */
function readFiles(dirname) {
    return new Promise((resolve, reject) => {
        fs.readdir(dirname, function (err, filenames) {
            if (err) return reject(err);
            promiseAllP(filenames, (filename, index, resolve, reject) => {
                fs.readFile(path.resolve(dirname, filename), 'utf-8', function (err, content) {
                    if (err) return reject(err);
                    return resolve({filename: filename, contents: content});
                });
            })
                .then(results => {
                    return resolve(results);
                })
                .catch(error => {
                    return reject(error);
                });
        });
    });
}

/**
 * Merged the rows from the array into a single string
 *
 * @param mergedColumns columns to merge
 * @returns {*}
 */
function mergeRowsToString(mergedColumns) {
    return mergedColumns.map(mergedColumns => mergedColumns.join(','))
}

/**
 * Writes the given string to the file.
 *
 * @param directory Directory to write.
 * @param content Content to save.
 */
function writeToFile(directory, content) {
    fs.writeFileSync(`${directory}/result.csv`, content, {encoding: 'utf8', flag: 'w'});
}

/**
 * Retrieves all subdirectories of the given directory.
 *
 * @param directory The directory to check.
 * @returns {string[]}
 */
function getSubDirectories(directory) {
    return fs.readdirSync(directory, {withFileTypes: true})
        .filter(dirent => dirent.isDirectory())
        .map(dirent => dirent.name)
}

const resultDirectory = './results'
const directories = getSubDirectories(resultDirectory)
const subSubDirectories = directories
    .map(d => `${resultDirectory}/${d}`)
    .flatMap(d => getSubDirectories(d).map(s => `${d}/${s}`))

subSubDirectories.forEach(dirName => {
    readFiles(dirName)
        .then(files => {
            if (files && files.size <= 0) {
                return;
            }

            const sorted = files
                .filter(value => value.filename !== 'result.csv')
                .sort((f1, f2) =>
                    +f1.filename.slice(8, -4).toString() - +f2.filename.slice(8, -4).toString()
                );

            const mergedFile = [];
            sorted.forEach((item, fileIndex) => {
                const content = item.contents
                    .split('\n')
                    .slice(0, -1);

                content.forEach((value, index) => {
                    mergedFile[fileIndex] = content;
                })
            });
            // transform rows into columns
            const mergedColumns = []

            const l = mergedFile[0].length
            for (let i = 0; i < l; i++) {
                mergedColumns[i] = mergedFile.map(d => d[i]);
            }

            const mergedRows = mergeRowsToString(mergedColumns)
            const mergedContent = mergedRows.join(os.EOL);
            writeToFile(dirName, mergedContent);
        })
        .catch(error => {
            console.error(error);
        });
});