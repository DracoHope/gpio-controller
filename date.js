// Referring to the NOde JS Documentation - https://nodejs.org/dist/latest-v12.x/docs/api/modules.html#modules_exports_shortcut
// Under the export shortcut section
// According to the Node JS documentation we can just use the "exports" keyword instead of "module.exports"
// using the Function Expression  to declare a function with a variable assign to it
exports.getDate = function (){
                const today = new Date();
                let currentDay = today.getDay();

                const options = {
                    weekday: "long", // means it will display "Sunday", "Saturday" ...etc
                    day: "numeric",
                    month: "long"   // means it will display "January", "December" ...etc
                };

                // We be still passing this Data to the Web Page but as an Object not just a value
                let day = today.toLocaleDateString("en-US", options);
                return day;
            };

module.exports.getDay = getDay;

function getDay() {
    //   The below code segment basically is to generate a Date with a specific Format  
    const today = new Date();
    let currentDay = today.getDay();

    const options = {
        weekday: "long", // means it will display "Sunday", "Saturday" ...etc
    };

    let day = today.toLocaleDateString("en-US", options);
    return day;
}

console.log(module.exports);