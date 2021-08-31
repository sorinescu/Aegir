// Adapted from https://github.com/homebrewing/brauhaus-beerxml/blob/master/src/import.coffee
export function parseBeerXML(xml) {
    let doc, fermentable, fermentableNode, fermentableProperty, mash, mashProperty, parser, recipe, recipeNode,
        recipeProperty, spice, spiceNode, spiceProperty, step, stepNode, stepProperty,
        _i, _j, _l, _len, _len1, _len10, _len11, _len3, _len4, _len5, _len6, _len9, _m, _n, _o, _r, _ref, _ref1, _ref10,
        _ref11, _ref12, _ref3, _ref4, _ref5, _ref6,
        _ref7, _s, _t;

    parser = new DOMParser();
    doc = parser.parseFromString(xml, 'text/xml');
    _ref = doc.documentElement.childNodes || [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        recipeNode = _ref[_i];
        if (recipeNode.nodeName.toLowerCase() !== 'recipe') {
            continue;
        }

        recipe = {
            fermentables: [],
            spices: [],
        };
        _ref1 = recipeNode.childNodes || [];
        for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
            recipeProperty = _ref1[_j];
            switch (recipeProperty.nodeName.toLowerCase()) {
                case 'name':
                    recipe.name = recipeProperty.textContent;
                    break;
                case 'brewer':
                    recipe.author = recipeProperty.textContent;
                    break;
                case 'batch_size':
                    recipe.batchSize = parseFloat(recipeProperty.textContent);
                    break;
                case 'boil_size':
                    recipe.boilSize = parseFloat(recipeProperty.textContent);
                    break;
                case 'efficiency':
                    recipe.mashEfficiency = parseFloat(recipeProperty.textContent);
                    break;
                case 'primary_age':
                    recipe.primaryDays = parseFloat(recipeProperty.textContent);
                    break;
                case 'primary_temp':
                    recipe.primaryTemp = parseFloat(recipeProperty.textContent);
                    break;
                case 'secondary_age':
                    recipe.secondaryDays = parseFloat(recipeProperty.textContent);
                    break;
                case 'secondary_temp':
                    recipe.secondaryTemp = parseFloat(recipeProperty.textContent);
                    break;
                case 'tertiary_age':
                    recipe.tertiaryDays = parseFloat(recipeProperty.textContent);
                    break;
                case 'tertiary_temp':
                    recipe.tertiaryTemp = parseFloat(recipeProperty.textContent);
                    break;
                case 'carbonation':
                    recipe.bottlingPressure = parseFloat(recipeProperty.textContent);
                    break;
                case 'carbonation_temp':
                    recipe.bottlingTemp = parseFloat(recipeProperty.textContent);
                    break;
                case 'age':
                    recipe.agingDays = parseFloat(recipeProperty.textContent);
                    break;
                case 'age_temp':
                    recipe.agingTemp = parseFloat(recipeProperty.textContent);
                    break;
                case 'fermentables':
                    _ref3 = recipeProperty.childNodes || [];
                    for (_l = 0, _len3 = _ref3.length; _l < _len3; _l++) {
                        fermentableNode = _ref3[_l];
                        if (fermentableNode.nodeName.toLowerCase() !== 'fermentable') {
                            continue;
                        }
                        fermentable = {};
                        _ref4 = fermentableNode.childNodes || [];
                        for (_m = 0, _len4 = _ref4.length; _m < _len4; _m++) {
                            fermentableProperty = _ref4[_m];
                            switch (fermentableProperty.nodeName.toLowerCase()) {
                                case 'name':
                                    fermentable.name = fermentableProperty.textContent;
                                    break;
                                case 'amount':
                                    fermentable.weight = parseFloat(fermentableProperty.textContent);
                                    break;
                                case 'yield':
                                    fermentable["yield"] = parseFloat(fermentableProperty.textContent);
                                    break;
                                case 'color':
                                    fermentable.color = parseFloat(fermentableProperty.textContent);
                                    break;
                                case 'add_after_boil':
                                    fermentable.late = fermentableProperty.textContent.toLowerCase() === 'true';
                            }
                        }
                        recipe.fermentables.push(fermentable);
                    }
                    break;
                case 'hops':
                case 'miscs':
                    _ref5 = recipeProperty.childNodes || [];
                    for (_n = 0, _len5 = _ref5.length; _n < _len5; _n++) {
                        spiceNode = _ref5[_n];
                        if ((_ref6 = spiceNode.nodeName.toLowerCase()) !== 'hop' && _ref6 !== 'misc') {
                            continue;
                        }
                        spice = {};
                        _ref7 = spiceNode.childNodes || [];
                        for (_o = 0, _len6 = _ref7.length; _o < _len6; _o++) {
                            spiceProperty = _ref7[_o];
                            switch (spiceProperty.nodeName.toLowerCase()) {
                                case 'name':
                                    spice.name = spiceProperty.textContent;
                                    break;
                                case 'amount':
                                    spice.weight = parseFloat(spiceProperty.textContent);
                                    break;
                                case 'alpha':
                                    spice.aa = parseFloat(spiceProperty.textContent);
                                    break;
                                case 'use':
                                    spice.use = spiceProperty.textContent;
                                    break;
                                case 'form':
                                    spice.form = spiceProperty.textContent;
                            }
                        }
                        recipe.spices.push(spice);
                    }
                    break;
                case 'mash':
                    mash = recipe.mash = {
                        steps: []
                    };
                    _ref10 = recipeProperty.childNodes || [];
                    for (_r = 0, _len9 = _ref10.length; _r < _len9; _r++) {
                        mashProperty = _ref10[_r];
                        switch (mashProperty.nodeName.toLowerCase()) {
                            case 'name':
                                mash.name = mashProperty.textContent;
                                break;
                            case 'grain_temp':
                                mash.grainTemp = parseFloat(mashProperty.textContent);
                                break;
                            case 'sparge_temp':
                                mash.spargeTemp = parseFloat(mashProperty.textContent);
                                break;
                            case 'ph':
                                mash.ph = parseFloat(mashProperty.textContent);
                                break;
                            case 'notes':
                                mash.notes = mashProperty.textContent;
                                break;
                            case 'mash_steps':
                                _ref11 = mashProperty.childNodes || [];
                                for (_s = 0, _len10 = _ref11.length; _s < _len10; _s++) {
                                    stepNode = _ref11[_s];
                                    if (stepNode.nodeName.toLowerCase() !== 'mash_step') {
                                        continue;
                                    }
                                    step = {};
                                    _ref12 = stepNode.childNodes || [];
                                    for (_t = 0, _len11 = _ref12.length; _t < _len11; _t++) {
                                        stepProperty = _ref12[_t];
                                        switch (stepProperty.nodeName.toLowerCase()) {
                                            case 'name':
                                                step.name = stepProperty.textContent;
                                                break;
                                            case 'type':
                                                step.type = stepProperty.textContent;
                                                break;
                                            case 'infuse_amount':
                                                step.infuseAmount = parseFloat(stepProperty.textContent);
                                                break;
                                            case 'step_temp':
                                                step.temp = parseFloat(stepProperty.textContent);
                                                break;
                                            case 'end_temp':
                                                step.endTemp = parseFloat(stepProperty.textContent);
                                                break;
                                            case 'step_time':
                                                step.time = parseFloat(stepProperty.textContent);
                                                break;
                                            case 'decoction_amt':
                                                step.decoctionAmount = parseFloat(stepProperty.textContent);
                                        }
                                    }
                                    mash.steps.push(step);
                                }
                        }
                    }
            }
        }
        return recipe;
    }
    return null;
}
