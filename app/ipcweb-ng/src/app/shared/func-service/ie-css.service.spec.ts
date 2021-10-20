import { TestBed } from '@angular/core/testing';

import { IeCssService } from './ie-css.service';

describe('IeCssService', () => {
  let service: IeCssService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(IeCssService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
