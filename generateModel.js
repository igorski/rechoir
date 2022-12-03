// this is a Node.js script that can conveniently generate a lot of the boilerplate
// code necessary to generate a model for your plugins adjustable parameters, exposing
// it as a public API to DAWs and generating all UI related code

// define your plugins controller model here
// NOTE: all values are floats and while these are usually normalized to the 0 - 1 range, any arbitrary value is supported
//
// format: {
//     name: String,             // used to calculate derived variable names from
//     descr: String,            // description (exposed to user via host)
//     unitDescr: String,        // meaningful description of the unit represented by this value (exposed to user via host)
//     value: {
//         min: String|Number, // minimum value accepted for this parameter (fractional values require .f suffix)
//         max: String|Number, // maximum value accepted for this parameter (fractional values require .f suffix)
//         def: String|Number, // optional, default value for this parameter, falls back to min (fractional values require .f suffix)
//         type: String,       // optional, defaults to float, accepts:
//                             // 'bool' where the value is either 0 or 1 (on/off)
//                             // 'percent' (multiplied by 100)
//     },
//     ui: {               // optional, when defined, will create entry in .uidesc
//         x: Number,      // x, y coordinates and width and height of control
//         y: Number,
//         w: Number,
//         h: Number.
//     },
//     normalizedDescr: Boolean, // optional, whether to display the value in the host normalized (otherwise falls back to 0 - 1 range), defaults to false
//     customDescr: String,      // optional, custom instruction used in controller.cpp to format value
// }
const gateSubdivisionFormatFn = `
            tmpValue = Igorski::Calc::gateSubdivision( valueNormalized );
            if ( tmpValue <= 0.5f ) {
                sprintf( text, "%.d measures", ( int ) ( 1.f / tmpValue ));
            } else if ( tmpValue == 1.f ) {
                sprintf( text, "1 measure" );
            } else if ( tmpValue == 4.f ) {
                sprintf( text, "Quarter note" );
            } else if ( tmpValue == 8.f || tmpValue == 16.f ) {
                sprintf( text, "%.fth note", tmpValue );
            } else {
                sprintf( text, "1/%.f measure", tmpValue );
            }`;

const MODEL = [
    {
        name: "delayTime",
        descr: "Delay time",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.25f", type: "percent" },
        ui: { x: 59, y: 155, w: 126, h: 128 },
        customDescr: `tmpValue = round( valueNormalized * 16 );
            switch (( int ) tmpValue ) {
                default:
                    sprintf( text, "%.f 16th notes", tmpValue );
                    break;
                case 0:
                    sprintf( text, "Slapback" );
                    break;
                case 1:
                    sprintf( text, "16th note" );
                    break;
                case 2:
                    sprintf( text, "8th note" );
                    break;
                case 4:
                    sprintf( text, "Quarter note" );
                    break;
                case 6:  // 3 8th notes
                case 10: // 5 8th notes
                case 14: // 7 8th notes
                    sprintf( text, "%.f 8th notes", tmpValue / 2 );
                    break;
                case 8:
                    sprintf( text, "Half measure" );
                    break;
                case 12:
                    sprintf( text, "3 quarter notes" );
                    break;
                case 16:
                    sprintf( text, "Full measure" );
                    break;
            }`
    },
    {
        name: "delayFeedback",
        descr: "Delay feedback",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.5f", type: "percent" },
        ui: { x: 219, y: 155, w: 126, h: 128 }
    },
    {
        name: "delayMix",
        descr: "Delay mix",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.5f", type: "percent" },
        ui: { x: 379, y: 155, w: 126, h: 128 }
    },
    {
        name: "pitchShift",
        descr: "Pitch shift amount",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.5f", type: "percent" },
        ui: { x: 677, y: 155, w: 126, h: 128 }
    },
    {
        name: "harmonize",
        descr: "Scale",
        value: { min: "0", max: "1", def: "0", type: "percent" },
        ui: { x: 836, y: 155, w: 126, h: 128 },
        customDescr: `if ( valueNormalized == 0 ) {
                sprintf( text, "Off" );
            } else {
                switch (( int ) round( 6.f * valueNormalized )) {
                    default:
                    case 0:
                        sprintf( text, "Neutral (2nd, 5th)" );
                        break;
                    case 1:
                        sprintf( text, "Major" );
                        break;
                    case 2:
                        sprintf( text, "Mixolydian" );
                        break;
                    case 3:
                        sprintf( text, "Augmented" );
                        break;
                    case 4:
                        sprintf( text, "Dorian" );
                        break;
                    case 5:
                        sprintf( text, "Minor" );
                        break;
                    case 6:
                        sprintf( text, "Diminished" );
                        break;
                }
            }`
    },
    {
        name: "reverb",
        descr: "Freeze",
        value: { min: "0", max: "1", def: "0", type: "bool" },
        ui: { x: 578, y: 195, w: 25, h: 40 }
    },
    {
        name: "decimator",
        descr: "Decimation",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.f", type: "percent" },
        ui: { x: 508, y: 386, w: 126, h: 128 },
    },
    {
        name: "filterCutoff",
        descr: "Filter cutoff",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.5f", type: "percent" },
        ui: { x: 679, y: 386, w: 126, h: 128 },
        customDescr: 'sprintf( text, "%.2d Hz", ( int ) (( Igorski::VST::FILTER_MAX_FREQ - Igorski::VST::FILTER_MIN_FREQ ) * valueNormalized ) + ( int ) Igorski::VST::FILTER_MIN_FREQ );'
    },
    {
        name: "filterResonance",
        descr: "Filter resonance",
        unitDescr: "%",
        value: { min: "0.f", max: "1.f", def: "0.5f", type: "percent" },
        ui: { x: 839, y: 386, w: 126, h: 128 }
    },
    {
        name: "syncChoir",
        descr: "Modulate scale",
        value: { min: "0", max: "1", def: "0", type: "bool" },
        ui: { x: 59, y: 425, w: 25, h: 40 }
    },
    // gate speeds are normalized 0 - 1 range values that translate to a 1 to 32 range (measure subdivisions)
    {
        name: "oddSpeed",
        descr: "Odd channel speed",
        unitDescr: "steps",
        value: { min: "0.f", max: "1.f", def: "0.35f", type: "percent" },
        ui: { x: 111, y: 386, w: 126, h: 128 },
        customDescr: gateSubdivisionFormatFn,
    },
    {
        name: "evenSpeed",
        descr: "Even channel speed",
        unitDescr: "steps",
        value: { min: "0.f", max: "1.f", def: "1.f", type: "percent" },
        ui: { x: 336, y: 386, w: 126, h: 128 },
        customDescr: gateSubdivisionFormatFn,
    },
    {
        name: "linkLFOs",
        descr: "Sync choir",
        value: { min: "0", max: "1", def: "1", type: "bool" },
        ui: { x: 278, y: 425, w: 25, h: 40 }
    },
];

// DO NOT CHANGE BELOW

const fs = require( "fs" );

const SOURCE_FOLDER      = "./src";
const RESOURCE_FOLDER    = "./resource";
const START_OF_OUTPUT_ID = "// --- AUTO-GENERATED START";
const END_OF_OUTPUT_ID   = "// --- AUTO-GENERATED END";
const PARAM_ID_INDEX     = 1; // start enum id for custom param ids

function replaceContent( fileData, lineContent = [], startId = START_OF_OUTPUT_ID, endId = END_OF_OUTPUT_ID ) {
    // first generate the appropriate regular expression
    // result with default values should equal /(\/\/ AUTO-GENERATED START)([\s\S]*?)(\/\/ AUTO-GENERATED END)/g
    const regex = new RegExp( `(${startId.replace(/\//g, '\\/')})([\\s\\S]*?)(${endId.replace(/\//g, '\\/')})`, 'g' );

    // find the currently existing data which will be replaced
    const dataToReplace = fileData.match( regex );
    if ( !dataToReplace ) {
        return fileData;
    }
    // format the output as a String
    const output = `${startId}\n\n${lineContent.join( "" )}
${endId}`;

    return fileData.replace( dataToReplace, output );
}

function getType( entry ) {
    return entry.value?.type || undefined;
}

function generateNamesForParam({ name }) {
    const pascalCased = `${name.charAt(0).toUpperCase()}${name.slice(1)}`;
    // the model name
    const model = `f${pascalCased}`;
    // param reflects the model name
    const param = `${name}Param`;
    // paramId is used to identify the UI controls linked to the model
    const paramId = `k${pascalCased}Id`;
    // saved is used to describe a temporary variable to retrieve saved states
    const saved = `saved${pascalCased}`;
    // toSave is used to describe a temporary variable used to save a state
    const toSave = `toSave${pascalCased}`;

    return {
        model,
        param,
        paramId,
        saved,
        toSave
    }
}

function generateParamIds() {
    const outputFile    = `${SOURCE_FOLDER}/paramids.h`;
    const fileData      = fs.readFileSync( outputFile, { encoding:'utf8', flag:'r' });
    const lines = [];

    MODEL.forEach(( entry, index ) => {
        const { descr } = entry;
        const { paramId } = generateNamesForParam( entry );
        const line = `    ${paramId} = ${( index + PARAM_ID_INDEX )},    // ${descr}\n`;
        lines.push( line );
    });
    fs.writeFileSync( outputFile, replaceContent( fileData, lines ));
}

function generateVstHeader() {
    const outputFile    = `${SOURCE_FOLDER}/vst.h`;
    const fileData      = fs.readFileSync( outputFile, { encoding:'utf8', flag:'r' });
    const lines = [];

    MODEL.forEach( entry => {
        const { descr, value } = entry;
        const { model } = generateNamesForParam( entry );
        const type = getType( entry );
        let line;
        if ( type === "bool" ) {
            line = `        bool ${model} = ${!!parseFloat( value.def ) > 0.5};    // ${descr}`;
        } else {
            line = `        float ${model} = ${value.def ?? value.min};    // ${descr}`;
        }
        lines.push( `${line}\n` );
    });
    fs.writeFileSync( outputFile, replaceContent( fileData, lines ));
}

function generateVstImpl() {
    const outputFile = `${SOURCE_FOLDER}/vst.cpp`;
    let fileData     = fs.readFileSync( outputFile, { encoding: "utf8", flag: "r" });

    const processLines = [];
    const setStateLines = [];
    const setStateApplyLines = [];
    const getStateLines = [];

    MODEL.forEach( entry => {
        const { model, paramId, saved, toSave } = generateNamesForParam( entry );
        const type = getType( entry );

        if ( type === "bool" ) {
            // 1. Rechoir::process
            processLines.push(`
                    case ${paramId}:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            ${model} = ( value > 0.5f );
                        break;\n`);

            // 2. Rechoir::setState

            setStateLines.push(`    int32 ${saved} = 0;
    if ( streamer.readInt32( ${saved} ) == false )
        return kResultFalse;\n\n`
            );
            setStateApplyLines.push(`    ${model} = ${saved} > 0;\n`); // assignment to model

            // 3. Rechoir::getState

            getStateLines.push(`    streamer.writeInt32( ${model} ? 1 : 0 );\n` );

        } else {

            // 1. Rechoir::process
            processLines.push(`
                    case ${paramId}:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            ${model} = ( float ) value;
                        break;\n`);

            // 2. Rechoir::setState

            setStateLines.push(`    float ${saved} = 0.f;
    if ( streamer.readFloat( ${saved} ) == false )
        return kResultFalse;\n\n`
            );
            setStateApplyLines.push(`    ${model} = ${saved};\n`); // assignment to model

            // 3. Rechoir::getState

            getStateLines.push(`    streamer.writeFloat( ${model} );\n` );
        }
    });

    let startId = '// --- AUTO-GENERATED PROCESS START';
    let endId   = '// --- AUTO-GENERATED PROCESS END';
    fileData = replaceContent( fileData, processLines, startId, endId );

    startId = '// --- AUTO-GENERATED SETSTATE START';
    endId   = '// --- AUTO-GENERATED SETSTATE END';
    fileData = replaceContent( fileData, setStateLines, startId, endId );

    startId = '// --- AUTO-GENERATED SETSTATE APPLY START';
    endId   = '// --- AUTO-GENERATED SETSTATE APPLY END';
    fileData = replaceContent( fileData, setStateApplyLines, startId, endId );

    startId = '// --- AUTO-GENERATED GETSTATE START';
    endId   = '// --- AUTO-GENERATED GETSTATE END';
    fileData = replaceContent( fileData, getStateLines, startId, endId );

    fs.writeFileSync( outputFile, fileData );
}

function generateController() {
    const outputFile = `${SOURCE_FOLDER}/ui/controller.cpp`;
    let fileData     = fs.readFileSync( outputFile, { encoding:'utf8', flag:'r' });

    const initLines = [];
    const setStateLines = [];
    const getParamLines = [];
    let line;

    MODEL.forEach( entry => {
        const { param, paramId, saved } = generateNamesForParam( entry );
        const { descr, unitDescr, normalizedDescr, customDescr } = entry;
        const type = getType( entry );
        let { min, max, def } = entry.value;
        if ( !def ) {
            def = min;
        }

        // 1. PluginController::initialize

        if ( type === "bool" ) {
            line = `
    parameters.addParameter(
        USTRING( "${descr}" ), ${min}, ${max}, ${def}, ParameterInfo::kCanAutomate, ${paramId}, unitId
    );\n\n`;
        } else {
            line = `    RangeParameter* ${param} = new RangeParameter(
        USTRING( "${descr}" ), ${paramId}, USTRING( "${unitDescr}" ),
        ${min}, ${max}, ${def},
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( ${param} );\n\n`;
        }
        initLines.push( line );

        // 2. PluginController::setComponentState

        if ( type === "bool" ) {
            setStateLines.push(`    int32 ${saved} = ${parseInt( def )};
    if ( streamer.readInt32( ${saved} ) == false )
        return kResultFalse;
    setParamNormalized( ${paramId}, ${saved} ? 1 : 0 );\n\n`
            );
        } else {
            setStateLines.push(`    float ${saved} = ${def || "0.f" };
    if ( streamer.readFloat( ${saved} ) == false )
        return kResultFalse;
    setParamNormalized( ${paramId}, ${saved} );\n\n`
            );
        }

        // 3. PluginController::getParamStringByValue
        // TODO: we can optimize this by grouping case values

        line = `        case ${paramId}:`;

        if ( customDescr ) {
           line += `
            ${customDescr}`;
        } else if ( type === "bool" ) {
            line += `
            sprintf( text, "%s", ( valueNormalized == 0 ) ? "Off" : "On" );`;
        } else if ( type === "percent" ) {
            line += `
            sprintf( text, "%.2d %%", ( int ) ( valueNormalized * 100.f ));`;
        } else if ( normalizedDescr ) {
            line += `
            sprintf( text, "%.2f ${unitDescr}", normalizedParamToPlain( tag, valueNormalized ));`;
        } else {
            line += `
            sprintf( text, "%.2f", ( float ) valueNormalized );`;
        }

        line += `
            Steinberg::UString( string, 128 ).fromAscii( text );
            return kResultTrue;\n\n`;

        getParamLines.push( line );
    });
    fileData = replaceContent( fileData, initLines );

    let startId = '// --- AUTO-GENERATED SETCOMPONENTSTATE START';
    let endId   = '// --- AUTO-GENERATED SETCOMPONENTSTATE END';
    fileData = replaceContent( fileData, setStateLines, startId, endId );

    startId = '// --- AUTO-GENERATED GETPARAM START';
    endId   = '// --- AUTO-GENERATED GETPARAM END';
    fileData = replaceContent( fileData, getParamLines, startId, endId );

    fs.writeFileSync( outputFile, fileData );
}

function generateUI() {
    const outputFile = `${RESOURCE_FOLDER}/plugin.uidesc`;
    let fileData     = fs.readFileSync( outputFile, { encoding:'utf8', flag:'r' });

    const controlLines = [];
    const tagLines     = [];

    MODEL.filter(entry => !!entry.ui)
         .forEach(( entry, index ) =>
    {
        const { x, y, w, h } = entry.ui;
        const { descr }      = entry;
        let { min, max, def, type } = entry.value;
        if ( !def ) {
            def = min;
        }

        const { param } = generateNamesForParam( entry );
        let control;

        if ( type === "bool" ) {
            control = `<view
              control-tag="Unit1::${param}" class="CCheckBox" origin="${x}, ${y}" size="${w}, ${h}"
              max-value="${max}" min-value="${min}" default-value="${def}"
              background-offset="0, 0" boxfill-color="~ TransparentCColor" autosize="bottom"
              boxframe-color="~ TransparentCColor" checkmark-color="~ WhiteCColor"
              draw-crossbox="false" autosize-to-fit="false" frame-width="1"
              mouse-enabled="true" opacity="1" round-rect-radius="0"
              title="" transparent="true" wants-focus="true" wheel-inc-value="0.1"
        />`;
        } else {
            control = `<view
              control-tag="Unit1::${param}" class="CKnob" origin="${x}, ${y}" size="${w}, ${h}"
              angle-range="270" angle-start="135" autosize="right top"
              background-offset="0, 0" circle-drawing="true" corona-color="#83d2ad" corona-dash-dot="false"
              corona-drawing="true" corona-from-center="false" corona-inset="5" corona-inverted="false"
              corona-line-cap-butt="false" corona-outline="true" corona-outline-width-add="0" default-value="${def}"
              handle-color="#83d2ad" handle-line-width="8" handle-shadow-color="~ #1e2239" max-value="${max}"
              min-value="${min}" mouse-enabled="true" opacity="1" skip-handle-drawing="true" transparent="false"
              value-inset="5" wants-focus="true" wheel-inc-value="0.1" zoom-factor="1.5"
        />`;
        }
        controlLines.push(`
        <!-- ${descr} -->
        ${control}` );

        tagLines.push(`        <control-tag name="Unit1::${param}" tag="${( index + PARAM_ID_INDEX )}" />\n` );
    });

    let startId = '<!-- AUTO-GENERATED CONTROLS START -->';
    let endId   = '<!-- AUTO-GENERATED CONTROLS END -->';
    fileData = replaceContent( fileData, controlLines, startId, endId );

    startId = '<!-- AUTO-GENERATED TAGS START -->';
    endId   = '<!-- AUTO-GENERATED TAGS END -->';
    fileData = replaceContent( fileData, tagLines, startId, endId );

    fs.writeFileSync( outputFile, fileData );
}

(function execute() {
    try {
        generateParamIds();
        generateVstHeader();
        generateVstImpl();
        generateController();
        generateUI();

        console.log( 'Successfully generated the plugin model.' );
    } catch ( e ) {
        console.error( 'Something went horribly wrong when generate the model.', e );
    }
})();
