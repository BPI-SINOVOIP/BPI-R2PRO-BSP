import { TestBed } from '@angular/core/testing';

import { LastNavService } from './last-nav.service';

describe('LastNavService', () => {
  let service: LastNavService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(LastNavService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
